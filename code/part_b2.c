#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define BUZZER_PIN PD6
#define COMPARE_STEP 71U

ISR(TIMER0_COMPA_vect)
{
    // Toggle PD6 at each compare match
    PORTD ^= (1 << BUZZER_PIN);

    // Schedule the next compare match, wrapping at 256
    OCR0A = (uint8_t)(OCR0A + COMPARE_STEP);
}

int main(void)
{
    cli();

    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    // Configure PD6 as an output
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    // Stop Timer0 and select Normal mode
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;

    OCR0A = COMPARE_STEP;

    // Clear any pending compare-match flag
    TIFR0 = (1 << OCF0A);

    // Enable the Timer0 Compare Match A interrupt
    TIMSK0 = (1 << OCIE0A);

    // Start Timer0 with a prescaler of 256
    TCCR0B = (1 << CS02);

    sei();

    while (1)
    {
        // The interrupt generates the waveform
    }
}