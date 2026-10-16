#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

static void adc_init(void)
{
    // Configure PC0 (ADC0) as an input without a pull-up
    DDRC &= ~(1 << PC0);
    PORTC &= ~(1 << PC0);

    // Use AVCC as the reference and select ADC0
    // Keep the result right-adjusted
    ADMUX = (1 << REFS0);

    // Use software-triggered single conversions
    ADCSRB = 0;

    // Disable the digital input buffer on ADC0
    DIDR0 |= (1 << ADC0D);

    // Enable the ADC with a prescaler of 128
    // ADC clock = 16 MHz / 128 = 125 kHz
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0);
}

static uint16_t adc_read(void)
{
    // Start a conversion
    ADCSRA |= (1 << ADSC);

    // Wait until the conversion finishes
    while (ADCSRA & (1 << ADSC))
    {
    }

    return ADC;
}

static uint8_t adc_to_duty(uint16_t adc_value)
{
    uint8_t segment;

    // Clamp readings outside the measured range
    if (adc_value <= 24U)
    {
        return 5;
    }

    if (adc_value >= 991U)
    {
        return 50;
    }

    // Divide the calibrated range into ten segments
    segment = (uint8_t)(
        (uint32_t)(adc_value - 24U) * 10UL / (991U - 24U)
    );

    // Convert segment 0-9 to duty cycle 5-50%
    return (uint8_t)((segment + 1U) * 5U);
}

int main(void)
{
    uint16_t adc_value;
    uint16_t voltage_mv;

    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    uart_init();
    adc_init();

    _delay_ms(10);

    // Discard the first reading after initialization
    (void)adc_read();

    printf("Part E: Photoresistor ADC test\r\n");

    while (1)
    {
        adc_value = adc_read();

        // Estimate voltage using a nominal 5000 mV reference
        voltage_mv =
            (uint16_t)((uint32_t)adc_value * 5000UL / 1024UL);

        printf("ADC: %u  Duty Cycle: %u%%  Voltage: %u.%03u V\r\n",
               (unsigned int)adc_value,
               (unsigned int)adc_to_duty(adc_value),
               (unsigned int)(voltage_mv / 1000U),
               (unsigned int)(voltage_mv % 1000U));

        _delay_ms(200);
    }
}