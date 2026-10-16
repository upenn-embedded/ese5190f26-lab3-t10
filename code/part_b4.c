#define F_CPU 16000000UL

#include <avr/io.h>

#define BUZZER_PIN PD6

int main(void)
{
    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    // Configure PD6 (OC0A) as an output
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    // Stop Timer0 before configuring it
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;

    // Set TOP for approximately 440 Hz on OC0A
    OCR0A = 142;

    // Phase Correct PWM, OCR0A as TOP, toggle OC0A
    TCCR0A = (1 << COM0A0) | (1 << WGM00);

    // Mode 5, prescaler 64
    TCCR0B = (1 << WGM02) |
             (1 << CS01) |
             (1 << CS00);

    while (1)
    {
        // Timer0 hardware generates the waveform
    }
}