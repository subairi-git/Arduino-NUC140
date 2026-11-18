#!/usr/bin/env python3
"""Inventory only: validate official NuMaker UNO archive SHA and list source paths.

The upstream binary is not executed, extracted into the repository, or
distributed with this branch. License inspection must precede any reuse.
"""
import hashlib
from pathlib import Path
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "docs/NUMAKER_OFFICIAL_INVENTORY.md"
TMP = ROOT / ".numaker_audit.tar.bz2"
URL = "https://raw.githubusercontent.com/OpenNuvoton/NuMaker_UNO/master/Arduino_nuvoton.tar.bz2"
EXPECTED = "29740e360ecad17d04a07be8555a85496ae3b73e43c8baca8fa30c9fb28421c2"

try:
    urllib.request.urlretrieve(URL, TMP)
    checksum = hashlib.sha256(TMP.read_bytes()).hexdigest()
    if checksum != EXPECTED:
        raise RuntimeError(f"Official NuMaker archive SHA-256 mismatch: {checksum}")
    with tarfile.open(TMP, mode="r:bz2") as archive:
        names = [m.name for m in archive if m.isfile()]
    interesting = [
        p for p in sorted(names)
        if any(w in p.lower() for w in
               ("arduino.h", "wiring", "hardware", "boards.txt",
                "platform.txt", "variant", "cores", "serial",
                "i2c", "spi", "adc", "license", "copying"))
    ]
    lines = [
        "# NuMaker UNO official archive source inventory",
        "",
        "Source: https://github.com/OpenNuvoton/NuMaker_UNO",
        "",
        f"SHA-256 validated: \`{checksum}\`",
        "",
        f"Total regular files: {len(names)}",
        f"Matching core/peripheral/variant/license paths: {len(interesting)}",
        "",
        "Archive listing only; no code was executed or redistributed.",
        "",
        "```text",
        *interesting[:600],
        "```",
        "",
        "If truncated, consult the original upstream archive.",
    ]
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {OUTPUT}")
finally:
    TMP.unlink(missing_ok=True)
