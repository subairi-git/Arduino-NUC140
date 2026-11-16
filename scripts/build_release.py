#!/usr/bin/env python3
"""Build an Arduino Boards Manager platform archive and its package index.

Arduino requires EXACTLY one top-level directory inside platform archives.
Version follows GitHub tag GITHUB_REF_NAME, or defaults to v0.1.1 locally.
"""
from pathlib import Path
import hashlib
import json
import os
import re
import tarfile

root = Path(__file__).resolve().parent.parent
ref_name = os.environ.get("GITHUB_REF_NAME", "v0.1.1")
if not re.fullmatch(r"v?\d+\.\d+\.\d+", ref_name):
    raise ValueError(f"Expected tag vMAJOR.MINOR.PATCH, got: {ref_name!r}")
version = ref_name.removeprefix("v")
repo = os.environ.get("GITHUB_REPOSITORY", "subairi-git/Arduino-NUC140")

source = root / "hardware" / "nuc140"
if not (source / "boards.txt").is_file() or not (source / "platform.txt").is_file():
    raise FileNotFoundError("hardware/nuc140 must contain boards.txt and platform.txt")

dist = root / "dist"
dist.mkdir(exist_ok=True)
archive_name = f"nuc140-{version}.tar.gz"
archive = dist / archive_name
archive_root = f"nuc140-{version}"

# Every file goes inside one top-level folder (required by Arduino Board Manager).
with tarfile.open(archive, "w:gz") as tar:
    for file in sorted(source.rglob("*")):
        if file.is_file():
            inner = file.relative_to(source).as_posix()
            tar.add(file, arcname=f"{archive_root}/{inner}")

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
package_index = {
    "packages": [{
        "name": "nuc140",
        "maintainer": "Community experimental core",
        "websiteURL": f"https://github.com/{repo}",
        "email": "",
        "help": {"online": f"https://github.com/{repo}#readme"},
        "platforms": [platform],
        "tools": [],
    }]
}
index_file = dist / "package_nuc140_index.json"
index_file.write_text(json.dumps(package_index, indent=2) + "\n", encoding="utf-8")
print(f"Created {archive} ({len(blob)} bytes)")
print(f"Created {index_file} (version {version})")
