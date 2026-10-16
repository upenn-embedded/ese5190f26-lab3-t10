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

    OCR0A = 70;

    // CTC mode, toggle OC0A on compare match
    TCCR0A = (1 << COM0A0) | (1 << WGM01);

    // Start Timer0 with a prescaler of 256
    TCCR0B = (1 << CS02);

    while (1)
    {
        // Timer0 hardware generates the waveform
    }
}