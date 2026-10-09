/* i2c.h */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C_WRITE           0
#define I2C_READ            1

typedef enum {
    I2C_OK = 0,
    I2C_ERROR_START,
    I2C_ERROR_ADDR_NACK,
    I2C_ERROR_DATA_NACK,
    I2C_ERROR_ARBITRATION
} i2c_status_t;

void i2c_init(uint32_t cpu_freq, uint32_t scl_freq);

i2c_status_t i2c_start(uint8_t address, uint8_t read);

void i2c_stop(void);

i2c_status_t i2c_write(uint8_t data);

uint8_t i2c_read_ack(void);

uint8_t i2c_read_nack(void);

i2c_status_t i2c_write_register(
    uint8_t address,
    uint8_t reg,
    uint8_t data
);

i2c_status_t i2c_read_register(
    uint8_t address,
    uint8_t reg,
    uint8_t *data
);

i2c_status_t i2c_write_register(
    uint8_t address,
    uint8_t reg,
    uint8_t data);

i2c_status_t i2c_read_burst(
    uint8_t address,
    uint8_t reg,
    uint8_t *data,
    uint8_t length);

#endif
