#!/usr/bin/env python3
"""Build an Arduino Boards Manager platform archive and its package index.

Arduino requires EXACTLY one top-level directory inside platform archives.
Version follows GitHub tag GITHUB_REF_NAME, or defaults to v0.1.4 locally.
"""
from pathlib import Path
import hashlib
import gzip
import json
import os
import re
import tarfile

root = Path(__file__).resolve().parent.parent
ref_name = os.environ.get("GITHUB_REF_NAME", "v0.1.4")
if not re.fullmatch(r"v?\d+\.\d+\.\d+", ref_name):
    raise ValueError(f"Expected tag vMAJOR.MINOR.PATCH, got: {ref_name!r}")
version = ref_name.removeprefix("v")
repo = os.environ.get("GITHUB_REPOSITORY", "subairi-git/Arduino-NUC140")

source = root / "hardware" / "nuc140"
if not (source / "boards.txt").is_file() or not (source / "platform.txt").is_file():
    raise FileNotFoundError("hardware/nuc140 must contain boards.txt and platform.txt")

if f"version={version}" not in (source / "platform.txt").read_text(encoding="utf-8").splitlines():
    raise ValueError("Tag release version must match hardware/nuc140/platform.txt")

dist = root / "dist"
dist.mkdir(exist_ok=True)
archive_name = f"nuc140-{version}.tar.gz"
archive = dist / archive_name
archive_root = f"nuc140-{version}"

# Every file goes inside one top-level folder (required by Arduino Board Manager).
# Normalize archive member metadata and gzip timestamp: reproducible SHA256 even
# across a new GitHub Actions checkout, so published index can be regenerated.
with archive.open("wb") as output:
    with gzip.GzipFile(filename="", fileobj=output, mode="wb", mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode="w|") as tar:
            for file in sorted(source.rglob("*")):
                if file.is_file():
                    inner = file.relative_to(source).as_posix()
                    member = tar.gettarinfo(str(file), arcname=f"{archive_root}/{inner}")
                    member.uid = member.gid = 0
                    member.uname = member.gname = ""
                    member.mtime = 0
                    with file.open("rb") as fd:
                        tar.addfile(member, fd)

blob = archive.read_bytes()
platform = {
    "name": "Nuvoton NUC140 (Experimental)",
    "architecture": "nuc140",
    "version": version,
    "category": "Contributed",
    "url": f"https://github.com/{repo}/releases/download/v{version}/{archive_name}",
    "archiveFileName": archive_name,
    "checksum": "SHA-256:" + hashlib.sha256(blob).hexdigest(),
    "size": str(len(blob)),
    "boards": [{"name": "Nu-LB-NUC140 V2.0 (NUC140VE3CN)"}],
    "toolsDependencies": [
        {"packager": "arduino", "name": "arm-none-eabi-gcc", "version": "7-2017q4"}
    ],
}
# Keep previously published versions in the generated index.
previous_platforms = []
previous_index_path = root / "package_nuc140_index.json"
if previous_index_path.is_file():
    old_index = json.loads(previous_index_path.read_text(encoding="utf-8"))
    for package in old_index.get("packages", []):
        if package.get("name") == "nuc140":
            previous_platforms = [
                p for p in package.get("platforms", [])
                if p.get("architecture") == "nuc140"
                and p.get("version") != version
            ]
            break

package_index = {
    "packages": [{
        "name": "nuc140",
        "maintainer": "Community experimental core",
        "websiteURL": f"https://github.com/{repo}",
        "email": "",
        "help": {"online": f"https://github.com/{repo}#readme"},
        "platforms": [platform] + previous_platforms,
        "tools": [],
    }]
}
index_file = dist / "package_nuc140_index.json"
index_file.write_text(json.dumps(package_index, indent=2) + "\n", encoding="utf-8")
print(f"Created {archive} ({len(blob)} bytes)")
print(f"Created {index_file} (version {version})")
