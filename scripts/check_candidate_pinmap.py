#!/usr/bin/env python3
"""Static checks for provisional NUC140 Arduino peripheral pin mapping."""
from pathlib import Path
import re

p = Path(__file__).resolve().parents[1] / "hardware/nuc140/variants/nuc140v2/peripheral_pins.h"
s = p.read_text(encoding="utf-8")
pins = {name: int(value) for name, value in re.findall(r"^#define\s+(PIN_\w+)\s+(\d+)\b", s, re.M)}
expected = {
    "PIN_SERIAL_RX": 16, "PIN_SERIAL_TX": 17,
    "PIN_WIRE_SDA": 8, "PIN_WIRE_SCL": 9,
    "PIN_SPI_SS": 40, "PIN_SPI_SCK": 41,
    "PIN_SPI_MISO": 42, "PIN_SPI_MOSI": 43,
    "PIN_ADC0": 0, "PIN_ADC1": 1, "PIN_ADC2": 2, "PIN_ADC3": 3,
}
assert pins == expected, f"Unexpected pin values: {pins}"
assert len(set(pins.values())) == len(pins), "Conflicting pins in candidate map"
assert 44 not in pins.values(), "PC12 onboard LED pin conflicts with peripheral candidate"
print("PASS: NUC140 candidate pin numbers match official BSP samples (static only)")
