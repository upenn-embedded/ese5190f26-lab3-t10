#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

#include "imu.h"
#include "uart.h"

#define IMU_ADDR 0x6B

#define BUZZER_PIN PD6
#define LED_PIN    PD3

#define IMU_MIN_VALUE (-10000L)
#define IMU_MAX_VALUE (10000L)

#define OCR_LOW_NOTE  60U
#define OCR_HIGH_NOTE 30U

static int32_t clamp_imu(int16_t raw)
{
    int32_t value = raw;

    if (value < IMU_MIN_VALUE)
    {
        value = IMU_MIN_VALUE;
    }

    if (value > IMU_MAX_VALUE)
    {
        value = IMU_MAX_VALUE;
    }

    return value;
}

static void buzzer_init(void)
{
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;

    OCR0A = 45;

    // Phase Correct PWM, OCR0A as TOP, toggle OC0A
    TCCR0A = (1 << COM0A0) | (1 << WGM00);

    // Timer0 prescaler: 64
    TCCR0B = (1 << WGM02) |
             (1 << CS01) |
             (1 << CS00);
}

static void led_init(void)
{
    DDRD |= (1 << LED_PIN);
    PORTD &= ~(1 << LED_PIN);

    TCCR2A = 0;
    TCCR2B = 0;
    TIMSK2 = 0;
    TCNT2 = 0;

    // Initial duty cycle: approximately 5%
    OCR2B = 12;

    // Fast PWM, TOP = 255, non-inverting OC2B
    TCCR2A = (1 << COM2B1) |
             (1 << WGM21) |
             (1 << WGM20);

    // Timer2 prescaler: 64
    TCCR2B = (1 << CS22);
}

static uint8_t x_to_ocr(int16_t raw_x)
{
    int32_t x = clamp_imu(raw_x);

    // Map -10000...10000 to OCR0A 60...30
    return (uint8_t)(
        OCR_LOW_NOTE -
        (x - IMU_MIN_VALUE) *
        (OCR_LOW_NOTE - OCR_HIGH_NOTE) /
        (IMU_MAX_VALUE - IMU_MIN_VALUE)
    );
}

static uint8_t y_to_duty(int16_t raw_y)
{
    int32_t y = clamp_imu(raw_y);

    // Map -10000...10000 to duty cycle 5...50%
    return (uint8_t)(
        5L +
        (y - IMU_MIN_VALUE) * 45L /
        (IMU_MAX_VALUE - IMU_MIN_VALUE)
    );
}

static void led_set_duty(uint8_t duty_percent)
{
    uint16_t high_ticks;

    // Non-inverting Fast PWM: duty = (OCR2B + 1) / 256
    high_ticks =
        ((uint16_t)duty_percent * 256U + 50U) / 100U;

    OCR2B = (uint8_t)(high_ticks - 1U);
}

int main(void)
{
    IMU_Data_t imu;
    uint8_t note_ocr;
    uint8_t duty_percent;

    // Use the 16 MHz CPU clock without division
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    uart_init();

    printf("Part G4: IMU pitch and brightness control\r\n");

    if (IMU_init(IMU_ADDR) != 0)
    {
        printf("IMU initialization failed\r\n");

        while (1)
        {
            _delay_ms(500);
        }
    }

    buzzer_init();
    led_init();

    printf("IMU initialized successfully\r\n");
    printf("ACC X controls pitch; ACC Y controls brightness\r\n");

    while (1)
    {
        if (IMU_checkNewData())
        {
            if (IMU_getAll(&imu) == 0)
            {
                note_ocr = x_to_ocr(imu.acc_x);
                duty_percent = y_to_duty(imu.acc_y);

                OCR0A = note_ocr;
                led_set_duty(duty_percent);

                printf(
                    "ACC X: %d  ACC Y: %d  "
                    "OCR0A: %u  Duty: %u%%\r\n",
                    (int)imu.acc_x,
                    (int)imu.acc_y,
                    (unsigned int)note_ocr,
                    (unsigned int)duty_percent
                );
            }
            else
            {
                printf("IMU read failed\r\n");
            }
        }

        _delay_ms(100);
    }
}