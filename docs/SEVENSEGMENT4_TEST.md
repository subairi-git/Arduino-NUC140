# Nu-LB-NUC140 V2.0 built-in 4-digit 7-segment (common anode)

User hardware feedback: the initial test produced garbled digits despite verified PE0–PE7 and PC4–PC7 pin groups. The cause was an incorrect assumption of conventional a–g bit ordering.

Revised example: `SevenSegment4_BoardTest.ino`. Board-specific raw segment words, from https://gist.github.com/mavif/d31d160ec7bebdfbe4e5 :

```
0: 0x82; 1: 0xEE; 2: 0x07; 3: 0x46; 4: 0x6A
5: 0x52; 6: 0x12; 7: 0xE6; 8: 0x02; 9: 0x62
```

Segment bus GPE0–GPE7 active LOW; column digit enable GPC4–GPC7 active HIGH. **GPC4 selects rightmost digit**, GPC7 leftmost. Each digit refreshes at 2 ms slots (125 full scans/s). Sequence: 1234, 8888, 0000, rolling counter. This revision **still requires hardware retest**.

Board pin groups confirmed in Nuvoton Nu-LB-NUC140 User Manual Rev 2.0 page 9.
