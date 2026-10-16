#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

#define TRIG_PIN PD7
#define ECHO_PIN PB0
#define BUZZER_PIN PD6

#define TIMEOUT_TICKS 2500U

// Distance limits in units of 0.1 cm
// Replace 20 with your measured minimum distance
// Measured distance limits in units of 0.1 cm
#define DIST_MIN_TENTHS 21U
#define DIST_MAX_TENTHS 1931U

#define OCR_HIGH_NOTE 30U
#define OCR_LOW_NOTE  60U

static void sensor_init(void)
{
    // Configure Trig as an output and Echo as an input
    DDRD |= (1 << TRIG_PIN);
    PORTD &= ~(1 << TRIG_PIN);

    DDRB &= ~(1 << ECHO_PIN);
    PORTB &= ~(1 << ECHO_PIN);

    // Timer1: Normal mode, rising-edge capture, prescaler 256
    TCCR1A = 0;
    TCCR1B = (1 << ICES1) | (1 << CS12);
    TIMSK1 = 0;
    TCNT1 = 0;
}

static void buzzer_init(void)
{
    // Configure PD6 (OC0A) as an output
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    // Stop Timer0 before configuring it
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;

    OCR0A = OCR_LOW_NOTE;

    // Phase Correct PWM, OCR0A as TOP, toggle OC0A
    TCCR0A = (1 << COM0A0) | (1 << WGM00);

    // Start Timer0 with a prescaler of 64
    TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
}

static uint8_t measure_echo(uint16_t *pulse_ticks)
{
    uint16_t wait_start;
    uint16_t rising_time;
    uint16_t falling_time;

    // Do not trigger while Echo is still high
    if (PINB & (1 << ECHO_PIN))
    {
        return 0;
    }

    // Prepare to capture the rising edge
    TCCR1B |= (1 << ICES1);
    TIFR1 = (1 << ICF1);
    wait_start = TCNT1;

    // Send a 10 us trigger pulse
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTD |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);

    // Wait for the rising edge
    while (!(TIFR1 & (1 << ICF1)))
    {
        if ((uint16_t)(TCNT1 - wait_start) >= TIMEOUT_TICKS)
        {
            return 0;
        }
    }

    rising_time = ICR1;

    // Prepare to capture the falling edge
    TCCR1B &= ~(1 << ICES1);
    TIFR1 = (1 << ICF1);

    // Wait for the falling edge
    while (!(TIFR1 & (1 << ICF1)))
    {
        if ((uint16_t)(TCNT1 - wait_start) >= TIMEOUT_TICKS)
        {
            return 0;
        }
    }

    falling_time = ICR1;
    *pulse_ticks = (uint16_t)(falling_time - rising_time);

    return *pulse_ticks != 0;
}

static uint8_t distance_to_ocr(uint16_t distance)
{
    // Clamp readings to the calibrated distance range
    if (distance <= DIST_MIN_TENTHS)
    {
        return OCR_HIGH_NOTE;
    }

    if (distance >= DIST_MAX_TENTHS)
    {
        return OCR_LOW_NOTE;
    }

    // Map distance linearly to OCR0A
    return (uint8_t)(
        OCR_HIGH_NOTE +
        (uint32_t)(distance - DIST_MIN_TENTHS) *
        (OCR_LOW_NOTE - OCR_HIGH_NOTE) /
        (DIST_MAX_TENTHS - DIST_MIN_TENTHS)
    );
}

int main(void)
{
    uint16_t pulse_ticks;
    uint16_t distance_tenths;
    uint8_t note_ocr;

    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    uart_init();
    sensor_init();

    _delay_ms(100);
    buzzer_init();

    printf("Part D1: Continuous mode\r\n");

    while (1)
    {
        if (measure_echo(&pulse_ticks))
        {
            // Each Timer1 tick is 16 us
            // Distance in 0.1 cm = ticks * 16 * 10 / 58
            distance_tenths =
                (uint16_t)((uint32_t)pulse_ticks * 160UL / 58UL);

            note_ocr = distance_to_ocr(distance_tenths);
            OCR0A = note_ocr;

            printf("Distance: %u.%u cm  OCR0A: %u\r\n",
                   (unsigned int)(distance_tenths / 10),
                   (unsigned int)(distance_tenths % 10),
                   (unsigned int)note_ocr);
        }
        else
        {
            // Keep the previous tone if no valid echo is received
            printf("Echo timeout / invalid reading\r\n");
        }

        _delay_ms(100);
    }
}