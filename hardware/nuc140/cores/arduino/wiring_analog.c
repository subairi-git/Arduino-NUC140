#include "Arduino.h"

/*
 * NUC140VE3CN ADC: polling single-ended conversions on PA0..PA7.
 * Hardware register locations and alternate-function masks are checked
 * against OpenNuvoton/NUC100BSP (NUC100Series.h, sys.h, adc.h).
 *
 * VR1 on Nu-LB-NUC140 V2.0 is wired to PA7/ADC7.
 * 12-bit native result: 0..4095; -1 on bad pin/timeout.
 *
 * The ADC clock uses HIRC/7 and does not alter HCLK, UART clock,
 * UART pin assignments, SysTick, or the Nu-Link upload mechanism.
 */
#define ADC_REG32(address) (*(volatile uint32_t *)(address))

#define REG_PROTECT    0x50000100UL
#define REG_GPA_MFP    0x50000030UL
#define REG_ALT_MFP    0x50000050UL
#define REG_ALT_MFP1   0x50000058UL
#define REG_PA_OFFD    0x50004004UL

#define REG_PWRCON     0x50000200UL
#define REG_APBCLK     0x50000208UL
#define REG_CLKSTATUS  0x5000020CUL
#define REG_CLKSEL1    0x50000214UL
#define REG_CLKDIV     0x50000218UL

#define REG_ADC_ADCR   0x400E0020UL
#define REG_ADC_ADCHER 0x400E0024UL
#define REG_ADC_ADSR   0x400E0030UL
#define REG_ADC_ADDR0  0x400E0000UL

#define ADC_WAIT_LIMIT 1000000UL

static uint8_t adc_initialized;

/* NUC100 BSP SYS_*_PAx_Msk masks for PA0..PA7. 
 * EBI_EN (ALT_MFP[11]) is shared; ADC pin configuration conflicts
 * with EBI pin-multiplexing, as on the underlying chip.
 */
static const uint32_t adc_alt_mfp_clear[8] = {
    0UL,
    (1UL << 11) | (1UL << 20),
    (1UL << 11) | (1UL << 19),
    (1UL << 11) | (1UL << 18),
    (1UL << 11) | (1UL << 17),
    (1UL << 11) | (1UL << 16),
    (1UL << 11),
    (1UL << 11) | (1UL << 2)
};

static const uint32_t adc_alt_mfp1_clear[8] = {
    (1UL << 2), (1UL << 3), (1UL << 0), (1UL << 1),
    (1UL << 7), (1UL << 8), (1UL << 5), (1UL << 6)
};

static int adc_prepare_pin(uint32_t channel)
{
    ADC_REG32(REG_PROTECT) = 0x59UL;
    ADC_REG32(REG_PROTECT) = 0x16UL;
    ADC_REG32(REG_PROTECT) = 0x88UL;
    if ((ADC_REG32(REG_PROTECT) & 1UL) == 0UL)
        return 0;

    if (!adc_initialized) {
        ADC_REG32(REG_PWRCON) |= (1UL << 2);
        uint32_t timeout = ADC_WAIT_LIMIT;
        while ((ADC_REG32(REG_CLKSTATUS) & (1UL << 4)) == 0UL && timeout)
            --timeout;
        if (timeout == 0UL) {
            ADC_REG32(REG_PROTECT) = 0UL;
            return 0;
        }

        ADC_REG32(REG_APBCLK) |= (1UL << 28);
        /* CLKSEL1 ADC_S = 3 (HIRC 22.1184 MHz). */
        ADC_REG32(REG_CLKSEL1) =
            (ADC_REG32(REG_CLKSEL1) & ~(3UL << 2)) | (3UL << 2);
        /* CLKDIV ADC_N = 6 => divisor 7. */
        ADC_REG32(REG_CLKDIV) =
            (ADC_REG32(REG_CLKDIV) & ~(0xFFUL << 16)) | (6UL << 16);
    }

    /* The core's PA(n)=n encoding maps directly to ADC channel n. */
    ADC_REG32(REG_ALT_MFP) &= ~adc_alt_mfp_clear[channel];
    ADC_REG32(REG_ALT_MFP1) &= ~adc_alt_mfp1_clear[channel];
    ADC_REG32(REG_GPA_MFP) |= (1UL << channel);

    ADC_REG32(REG_PROTECT) = 0UL;
    ADC_REG32(REG_PA_OFFD) |= (1UL << (16UL + channel));

    if (!adc_initialized) {
        /* ADEN=1; single-ended, single conversion mode; interrupts off. */
        ADC_REG32(REG_ADC_ADCR) = 1UL;
        adc_initialized = 1U;
    }
    return 1;
}

int analogRead(uint32_t pin)
{
    if (pin > 7UL)
        return -1;
    if (!adc_prepare_pin(pin))
        return -1;

    /* Select just one external ADC channel. */
    ADC_REG32(REG_ADC_ADCHER) = (1UL << pin);

    /* ADF is write-one-to-clear; ADST starts single conversion. */
    ADC_REG32(REG_ADC_ADSR) = 1UL;
    ADC_REG32(REG_ADC_ADCR) |= (1UL << 11);

    uint32_t timeout = ADC_WAIT_LIMIT;
    while ((ADC_REG32(REG_ADC_ADSR) & 1UL) == 0UL && timeout)
        --timeout;
    if (timeout == 0UL)
        return -1;

    return (int)(ADC_REG32(REG_ADC_ADDR0 + 4UL * pin) & 0x0FFFUL);
}
