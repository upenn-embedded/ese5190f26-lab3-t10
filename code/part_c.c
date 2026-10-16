#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

#define TRIG_PIN       PD7
#define ECHO_PIN       PB0
#define TIMEOUT_TICKS  2500U   // 40 ms, at 16 us per tick

static void sensor_init(void)
{
    // PD7: Trigger output, initially low
    DDRD |= (1 << TRIG_PIN);
    PORTD &= ~(1 << TRIG_PIN);

    // PB0: Echo input, no internal pull-up
    DDRB &= ~(1 << ECHO_PIN);
    PORTB &= ~(1 << ECHO_PIN);

    // Timer1: Normal mode, rising-edge capture, prescaler 256
    TCCR1A = 0;
    TCCR1B = (1 << ICES1) | (1 << CS12);
    TIMSK1 = 0;
    TCNT1 = 0;
}

static uint8_t measure_echo(uint16_t *pulse_ticks)
{
    uint16_t wait_start;
    uint16_t rising_time;
    uint16_t falling_time;

    // Do not trigger if the previous Echo is still high
    if (PINB & (1 << ECHO_PIN)) {
        return 0;
    }

    // Prepare to capture rising edge
    TCCR1B |= (1 << ICES1);
    TIFR1 = (1 << ICF1);

    wait_start = TCNT1;

    // Send Trigger pulse
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);

    PORTD |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);

    // Wait for rising edge
    while (!(TIFR1 & (1 << ICF1))) {
        if ((uint16_t)(TCNT1 - wait_start) >= TIMEOUT_TICKS) {
            return 0;
        }
    }

    rising_time = ICR1;

    // Switch to falling-edge capture
    TCCR1B &= ~(1 << ICES1);
    TIFR1 = (1 << ICF1);

    // Wait for falling edge
    while (!(TIFR1 & (1 << ICF1))) {
        if ((uint16_t)(TCNT1 - wait_start) >= TIMEOUT_TICKS) {
            return 0;
        }
    }

    falling_time = ICR1;
    *pulse_ticks = (uint16_t)(falling_time - rising_time);

    return (*pulse_ticks != 0);
}

int main(void)
{
    uint16_t pulse_ticks;
    uint16_t distance_tenths;

    // Use default 16 MHz board clock without system division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    uart_init();
    sensor_init();

    _delay_ms(100);
    printf("Lab 3 Part C ready\r\n");

    while (1) {
        if (measure_echo(&pulse_ticks)) {

            // Distance multiplied by 10, to display one decimal
            distance_tenths =
                (uint16_t)((uint32_t)pulse_ticks * 160UL / 58UL);

            printf("Ticks: %u  Distance: %u.%u cm\r\n",
                   (unsigned int)pulse_ticks,
                   (unsigned int)(distance_tenths / 10),
                   (unsigned int)(distance_tenths % 10));

        } else {
            printf("Echo timeout / invalid reading\r\n");
        }

        _delay_ms(100);
    }
}