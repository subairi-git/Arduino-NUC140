#!/usr/bin/env python3
"""Validate the generated Arduino Boards Manager package archive and index."""
from pathlib import Path
import hashlib
import json
import tarfile

root = Path(__file__).resolve().parent.parent
dist = root / "dist"
index = json.loads((dist / "package_nuc140_index.json").read_text(encoding="utf-8"))
platform = index["packages"][0]["platforms"][0]
archive = dist / platform["archiveFileName"]
assert archive.is_file(), f"Missing archive: {archive}"
content = archive.read_bytes()
assert platform["size"] == str(len(content)), "Archive size mismatch"
assert platform["checksum"] == "SHA-256:" + hashlib.sha256(content).hexdigest(), "SHA256 mismatch"

with tarfile.open(archive, "r:gz") as tar:
    paths = [member.name for member in tar.getmembers() if member.isfile()]
assert paths, "Archive contains no files"
roots = {path.split("/")[0] for path in paths}
assert len(roots) == 1, f"Expected one top-level folder, got {sorted(roots)}"
archive_root = next(iter(roots))
assert archive_root == f"nuc140-{platform['version']}", "Wrong archive root folder"
assert f"{archive_root}/boards.txt" in paths, "Missing boards.txt"
assert f"{archive_root}/platform.txt" in paths, "Missing platform.txt"
assert f"{archive_root}/cores/arduino/Arduino.h" in paths, "Missing Arduino core header"
assert f"{archive_root}/variants/nuc140v2/flash.ld" in paths, "Missing linker script"
assert f"{archive_root}/tools/nulink-upload.ps1" in paths, "Missing Nu-Link uploader"
print("PASS: archive has one root, required files, correct SHA-256 and size")
