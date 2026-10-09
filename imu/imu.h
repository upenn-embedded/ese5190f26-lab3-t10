/* 
 * File:   imu.h
 *  Supported IMU(s): LSM6DSO
 * 
 * Author: James Steeman
 * Modified By: Praise Ndlovu
 * Created on February 22, 2025, 2:20 AM
 * Updated Oct. 1, 2025
 */

#ifndef IMU_H
#define IMU_H

#include <stdint.h>

typedef struct
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

    int16_t temperature;

} IMU_Data_t;

int IMU_init(uint8_t addr);

/*
 * Reads temperature, gyro, and accelerometer data
 * into the supplied IMU_Data_t structure.
 *
 * Returns 0 on success.
 */
int IMU_getAll(IMU_Data_t *imu);

/*
 * Reads temperature, gyroscope, and accelerometer data
 * from the IMU and stores the raw sensor values in the
 * supplied IMU_Data_t structure.
 *
 * Parameters:
 *   imu - Pointer to an IMU_Data_t structure where the
 *         sensor data will be stored.
 *
 * Returns:
 *   0 on success.
 *   Non-zero value if the I2C transaction fails.
 */
int IMU_getAll(IMU_Data_t *imu);


/*
 * Reads the raw X-axis accelerometer value.
 *
 * Returns:
 *   Signed 16-bit raw accelerometer X-axis measurement.
 */
int16_t IMU_getAccX(void);


/*
 * Reads the raw Y-axis accelerometer value.
 *
 * Returns:
 *   Signed 16-bit raw accelerometer Y-axis measurement.
 */
int16_t IMU_getAccY(void);


/*
 * Reads the raw Z-axis accelerometer value.
 *
 * Returns:
 *   Signed 16-bit raw accelerometer Z-axis measurement.
 */
int16_t IMU_getAccZ(void);


/*
 * Reads the raw X-axis gyroscope value.
 *
 * Returns:
 *   Signed 16-bit raw gyroscope X-axis measurement.
 */
int16_t IMU_getGyroX(void);


/*
 * Reads the raw Y-axis gyroscope value.
 *
 * Returns:
 *   Signed 16-bit raw gyroscope Y-axis measurement.
 */
int16_t IMU_getGyroY(void);


/*
 * Reads the raw Z-axis gyroscope value.
 *
 * Returns:
 *   Signed 16-bit raw gyroscope Z-axis measurement.
 */
int16_t IMU_getGyroZ(void);


/*
 * Reads the raw temperature sensor value.
 *
 * Returns:
 *   Signed 16-bit raw temperature measurement.
 */
int16_t IMU_getTemp(void);


/*
 * Checks whether new accelerometer or gyroscope data
 * is available from the IMU.
 *
 * Returns:
 *   Non-zero if new sensor data is available.
 *   0 if no new data is available.
 */
int IMU_checkNewData(void);

#endif /* IMU_H */
