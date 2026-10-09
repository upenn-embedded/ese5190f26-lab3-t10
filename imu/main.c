// This file demonstrates using the libraries for IMU over I2C.

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "imu.h"
#include "uart.h"

#define IMU_ADDR 0x6B

int main(void)
{
    IMU_Data_t imu;

    uart_init();

    printf("Starting IMU test\r\n");

    int init_status = IMU_init(IMU_ADDR);

    if (init_status != 0)
    {
        printf("IMU initialization failed\r\n");

        while (1)
        {
            _delay_ms(500);
        }
    }

    printf("IMU initialized successfully\r\n");

    while (1)
    {
        /*
         * Check whether the IMU has new sensor data.
         */
        if (IMU_checkNewData())
        {
            /*
             * Read all sensor values in one burst.
             */
            int status = IMU_getAll(&imu);

            if (status == 0)
            {
                printf("---- Burst Read ----\r\n");

                printf(
                    "ACC  X: %d Y: %d Z: %d\r\n",
                    imu.acc_x,
                    imu.acc_y,
                    imu.acc_z
                );

                printf(
                    "GYRO X: %d Y: %d Z: %d\r\n",
                    imu.gyro_x,
                    imu.gyro_y,
                    imu.gyro_z
                );

                printf(
                    "TEMP: %d\r\n",
                    imu.temperature
                );
            }
            else
            {
                printf(
                    "IMU_getAll failed, status = %d\r\n",
                    status
                );
            }

            /*
             * Test the individual register-reading functions.
             */
            int16_t acc_x = IMU_getAccX();
            int16_t acc_y = IMU_getAccY();
            int16_t acc_z = IMU_getAccZ();

            int16_t gyro_x = IMU_getGyroX();
            int16_t gyro_y = IMU_getGyroY();
            int16_t gyro_z = IMU_getGyroZ();

            int16_t temperature = IMU_getTemp();

            printf("---- Individual Reads ----\r\n");

            printf(
                "ACC  X: %d Y: %d Z: %d\r\n",
                acc_x,
                acc_y,
                acc_z
            );

            printf(
                "GYRO X: %d Y: %d Z: %d\r\n",
                gyro_x,
                gyro_y,
                gyro_z
            );

            printf(
                "TEMP: %d\r\n",
                temperature
            );

            printf("\r\n");
        }
        else
        {
            printf("No new IMU data\r\n");
        }

        _delay_ms(500);
    }

    return 0;
}
