#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

/* Pin definitions */
#define TRIG_PIN   PD7
#define ECHO_PIN   PB0
#define BUZZER_PIN PD6
#define BUTTON_PIN PB1

/* Timer1 tick = 16 us; timeout = 40 ms */
#define TIMEOUT_TICKS 2500U

/* Measured distance limits in units of 0.1 cm */
#define DIST_MIN_TENTHS 21U
#define DIST_MAX_TENTHS 1931U

/* Timer0 TOP values for the highest and lowest notes */
#define OCR_HIGH_NOTE 30U
#define OCR_LOW_NOTE  60U

static void sensor_init(void)
{
    // Configure Trig as an output and initially set it low
    DDRD |= (1 << TRIG_PIN);
    PORTD &= ~(1 << TRIG_PIN);

    // Configure Echo as an input without a pull-up
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

    // Phase Correct PWM, Mode 5: OCR0A is TOP
    // Toggle OC0A on compare match
    TCCR0A = (1 << COM0A0) | (1 << WGM00);

    // Start Timer0 with a prescaler of 64
    TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
}

static void button_init(void)
{
    // Configure PB1 as an input with an internal pull-up
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB |= (1 << BUTTON_PIN);
}

static uint8_t button_pressed(void)
{
    static uint8_t previous_state = 1;
    uint8_t current_state;

    current_state = (PINB & (1 << BUTTON_PIN)) ? 1U : 0U;

    if (current_state != previous_state)
    {
        // Debounce before accepting a state change
        _delay_ms(20);

        current_state = (PINB & (1 << BUTTON_PIN)) ? 1U : 0U;

        if (current_state != previous_state)
        {
            previous_state = current_state;

            if (current_state == 0)
            {
                // Report a new button press
                printf("Button PRESSED: PB1 = 0\r\n");
                return 1;
            }
            else
            {
                // Report a button release
                printf("Button RELEASED: PB1 = 1\r\n");
            }
        }
    }

    return 0;
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

    // Unsigned subtraction handles Timer1 counter wraparound
    *pulse_ticks = (uint16_t)(falling_time - rising_time);

    return *pulse_ticks != 0;
}

static uint8_t distance_to_ocr(uint16_t distance)
{
    uint16_t distance_range;
    uint32_t scaled_value;

    // Clamp readings to the measured distance range
    if (distance <= DIST_MIN_TENTHS)
    {
        return OCR_HIGH_NOTE;
    }

    if (distance >= DIST_MAX_TENTHS)
    {
        return OCR_LOW_NOTE;
    }

    distance_range = DIST_MAX_TENTHS - DIST_MIN_TENTHS;

    // Map distance linearly to OCR0A
    scaled_value =
        (uint32_t)(distance - DIST_MIN_TENTHS) *
        (OCR_LOW_NOTE - OCR_HIGH_NOTE);

    // Round to the nearest integer OCR value
    return (uint8_t)(
        OCR_HIGH_NOTE +
        (scaled_value + distance_range / 2U) / distance_range
    );
}

static uint8_t distance_to_discrete_ocr(uint16_t distance)
{
    // Notes from nearest to farthest: C7, B6, A6, G6, F6, E6, D6, C6
    static const uint8_t note_ocr[8] = {
        30, 32, 36, 40, 45, 47, 53, 60
    };

    uint8_t index;

    // Clamp readings outside the measured range
    if (distance <= DIST_MIN_TENTHS)
    {
        return note_ocr[0];
    }

    if (distance >= DIST_MAX_TENTHS)
    {
        return note_ocr[7];
    }

    // Divide the measured distance range into eight equal bins
    index = (uint8_t)(
        (uint32_t)(distance - DIST_MIN_TENTHS) * 8UL /
        (DIST_MAX_TENTHS - DIST_MIN_TENTHS)
    );

    return note_ocr[index];
}

int main(void)
{
    uint16_t pulse_ticks;
    uint16_t distance_tenths;
    uint8_t note_ocr;

    // 0 = continuous mode, 1 = discrete mode
    uint8_t discrete_mode = 0;

    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    uart_init();
    sensor_init();
    button_init();

    _delay_ms(100);
    buzzer_init();

    printf("Part D2: Continuous mode\r\n");

    while (1)
    {
        // Toggle mode once for each debounced button press
        if (button_pressed())
        {
            discrete_mode ^= 1U;

            if (discrete_mode)
            {
                printf("Switched to discrete mode\r\n");
            }
            else
            {
                printf("Switched to continuous mode\r\n");
            }
        }

        if (measure_echo(&pulse_ticks))
        {
            // Convert Timer1 ticks to distance in units of 0.1 cm
            distance_tenths =
                (uint16_t)((uint32_t)pulse_ticks * 160UL / 58UL);

            if (discrete_mode)
            {
                note_ocr = distance_to_discrete_ocr(distance_tenths);
            }
            else
            {
                note_ocr = distance_to_ocr(distance_tenths);
            }

            OCR0A = note_ocr;

            printf("Mode: %s  Distance: %u.%u cm  OCR0A: %u\r\n",
                   discrete_mode ? "Discrete" : "Continuous",
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