# NUC140RTC — Internal RTC (preview)

Arduino-style library for NUC140VE3CN on-chip RTC, using a 32.768 kHz low speed crystal X1 already on Nu-LB-NUC140 V2.0. No DS1307/DS3231 module, no GPIO pins needed.

## Status
- Peripheral register implementation based on Nuvoton NUC100BSP (rtc.h, rtc.c, NUC100Series.h).
- ARM Cortex-M0 cross-compilation tested; **hardware RTC still needs user validation**.
- RTC clock initialization and calendar/time setting, 24-hour mode, 2000–2099, weekday 0..6.
- begin() enables LXT and RTC clock, checks crystal status (up to 5 seconds), initializes only if not already active; preserves existing time on MCU reset if RTC remains powered.
- setDateTime explicit; getDateTime attempts a consistent read across second rollover and validates BCD.
- Alarm and interrupts intentionally out of scope.

## API

    #include <NUC140RTC.h>
    void setup() {
      Serial.begin(115200);
      RTCClock.begin();
      RTCClock.setDateTime(2026, 11, 19, 15, 31, 0); // SET ONLY ONCE!
    }
    void loop() {
      RTCDateTime t;
      if (RTCClock.getDateTime(t)) Serial.println(t.second);
      delay(1000);
    }

RTCClock.begin() does not change the date/time. Use the RTC_Serial_Clock example and enter S YYYY-MM-DD HH:MM:SS in Arduino Serial Monitor to set it. Reboot with MCU RESET to confirm the RTC runs separately from millis().

The example RTC_Set_Once initializes a fixed demonstration timestamp on every boot; edit it to your time or disable the flag after the first test. The RTC_LCD_Clock example displays RTC time on NUC140LCD v0.1.1.

## Hardware / power

- LXT: board X1 32.768 kHz quartz crystal. CLK.PWRCON bit1, CLK.CLKSTATUS bit1.
- APBCLK RTC enable bit1, RTC base 0x40008000.
- Standard key for INIT 0xA5EB1357, access key 0xA965.
- RTC date/time in packed BCD (CLR, TLR), TSSR bit0 = 24-hour mode, DWR Sunday=0.
- After MCU reset the running RTC may persist; **after complete power loss**, continuous operation depends on backup V_BAT power. Verify the board's battery/backup connection before assuming retention.

## Install current manually patched Boards Manager package

Run INSTALL-RTC.cmd from a completely extracted ZIP. Only the libraries/NUC140RTC directory is copied into the installed NUC140 core version 0.1.2. Existing Serial, ADC, Wire I2C1, SPI2 and SPI3 LCD drivers are untouched.

## Official references

- https://github.com/OpenNuvoton/NUC100BSP
- https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
