#!/usr/bin/env python3
"""Validate the generated Arduino Boards Manager package archive and index."""
from pathlib import Path
import hashlib
import json
import tarfile
import re

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
with tarfile.open(archive, "r:gz") as t:
    members = {m.name: m for m in t.getmembers()}
    ptxt = t.extractfile(members[f"{archive_root}/platform.txt"]).read().decode("utf-8")
    uploader = t.extractfile(members[f"{archive_root}/tools/nulink-upload.ps1"]).read().decode("utf-8")
    cv = t.extractfile(members[f"{archive_root}/cores/arduino/core_version.h"]).read().decode("utf-8")
assert "0.1.3-experimental" in cv
assert "Invoke-NuLink -Arguments @('-l')" not in uploader, "Old NuLink listing blocker remains"
assert "Invoke-NuLink -Arguments @('-p')" in uploader, "Missing device identity check"
assert "recipe.size.regex=" in ptxt and "recipe.size.regex.data=" in ptxt
cfg = dict(line.split("=", 1) for line in ptxt.splitlines() if line and not line.startswith("#") and "=" in line)
assert cfg["version"] == platform["version"], "Platform/version mismatch"
example = ".vectors 192 0\n.text 356 192\n.init_array 4 548\n.data 8 536870912\n.bss 16 536870920\n"
flash_rx = re.compile(cfg["recipe.size.regex"])
ram_rx = re.compile(cfg["recipe.size.regex.data"])
flash = sum(int(m.group(1)) for line in example.splitlines() if (m := flash_rx.search(line)))
ram = sum(int(m.group(1)) for line in example.splitlines() if (m := ram_rx.search(line)))
assert (flash, ram) == (560, 24), f"Size regex failed: {flash}, {ram}"
print("PASS: archive has one root, required files, correct SHA-256 and size")
print("PASS: NuLink -p hotfix and Flash/SRAM regex")
