#ifndef LPS22HB_H
#define LPS22HB_H

#include <stdint.h>

#define LPS22HB_ADDRESS              0x5D
#define LPS22HB_ADDRESS_ALT          0x5C

#define LPS22HB_WHO_AM_I             0x0F
#define LPS22HB_WHO_AM_I_VALUE       0xB1

#define LPS22HB_CTRL_REG1            0x10
#define LPS22HB_CTRL_REG2            0x11
#define LPS22HB_RES_CONF             0x1A
#define LPS22HB_STATUS               0x27
#define LPS22HB_PRESS_OUT_XL         0x28
#define LPS22HB_TEMP_OUT_L           0x2B

#define LPS22HB_CTRL_REG1_CONFIG     0x30
#define LPS22HB_CTRL_REG2_CONFIG     0x10

#define LPS22HB_PRESSURE_LSB         4096.0f
#define LPS22HB_TEMPERATURE_LSB      100.0f

#define LPS22HB_SEA_LEVEL_PRESSURE   1013.25f
#define LPS22HB_ALTITUDE_FACTOR      44330.0f
#define LPS22HB_ALTITUDE_EXPONENT    0.1902949f

typedef uint8_t (*lps22hb_read_byte_t)(uint8_t address, uint8_t reg, uint8_t *data);
typedef uint8_t (*lps22hb_read_bytes_t)(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);
typedef uint8_t (*lps22hb_write_byte_t)(uint8_t address, uint8_t reg, uint8_t data);
typedef uint8_t (*lps22hb_write_bytes_t)(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);

void LPS22HB_RegisterBus(lps22hb_read_byte_t readByte, lps22hb_read_bytes_t readBytes,
                         lps22hb_write_byte_t writeByte, lps22hb_write_bytes_t writeBytes);
uint8_t LPS22HB_Init(void);
uint8_t LPS22HB_GetAddress(void);
uint8_t LPS22HB_ReadPressure(float *pressure);
uint8_t LPS22HB_ReadTemperature(float *temperature);
uint8_t LPS22HB_SetGroundReference(void);
float LPS22HB_CalculateAltitude(float pressure);

#endif
