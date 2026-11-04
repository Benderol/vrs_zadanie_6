#ifndef HTS221_H
#define HTS221_H

#include <stdint.h>

#define HTS221_ADDRESS               0x5F

#define HTS221_WHO_AM_I              0x0F
#define HTS221_WHO_AM_I_VALUE        0xBC

#define HTS221_AV_CONF               0x10
#define HTS221_CTRL_REG1             0x20
#define HTS221_CTRL_REG2             0x21
#define HTS221_STATUS_REG            0x27
#define HTS221_HUMIDITY_OUT_L        0x28
#define HTS221_TEMP_OUT_L            0x2A
#define HTS221_CALIBRATION_START     0x30
#define HTS221_CALIBRATION_SIZE      16

#define HTS221_AUTO_INCREMENT        0x80

#define HTS221_CTRL_REG1_CONFIG      0x87
#define HTS221_AV_CONF_CONFIG        0x1B

typedef uint8_t (*hts221_read_byte_t)(uint8_t address, uint8_t reg, uint8_t *data);
typedef uint8_t (*hts221_read_bytes_t)(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);
typedef uint8_t (*hts221_write_byte_t)(uint8_t address, uint8_t reg, uint8_t data);
typedef uint8_t (*hts221_write_bytes_t)(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length);

void HTS221_RegisterBus(hts221_read_byte_t readByte, hts221_read_bytes_t readBytes,
                        hts221_write_byte_t writeByte, hts221_write_bytes_t writeBytes);
uint8_t HTS221_Init(void);
uint8_t HTS221_ReadHumidity(float *humidity);
uint8_t HTS221_ReadTemperature(float *temperature);
float HTS221_Interpolate(int16_t raw, int16_t raw0, int16_t raw1, float value0, float value1);

#endif
