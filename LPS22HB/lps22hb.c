#include "lps22hb.h"
#include <math.h>

static lps22hb_read_byte_t busReadByte = 0;
static lps22hb_read_bytes_t busReadBytes = 0;
static lps22hb_write_byte_t busWriteByte = 0;
static lps22hb_write_bytes_t busWriteBytes = 0;

static uint8_t deviceAddress = LPS22HB_ADDRESS;
static float groundPressure = LPS22HB_SEA_LEVEL_PRESSURE;

void LPS22HB_RegisterBus(lps22hb_read_byte_t readByte, lps22hb_read_bytes_t readBytes,
                         lps22hb_write_byte_t writeByte, lps22hb_write_bytes_t writeBytes)
{
  busReadByte = readByte;
  busReadBytes = readBytes;
  busWriteByte = writeByte;
  busWriteBytes = writeBytes;
}

uint8_t LPS22HB_GetAddress(void)
{
  return deviceAddress;
}

static uint8_t LPS22HB_IsPresent(uint8_t address)
{
  uint8_t id = 0;

  if(!busReadByte(address, LPS22HB_WHO_AM_I, &id))
  {
    return 0;
  }

  return (id == LPS22HB_WHO_AM_I_VALUE);
}

static uint8_t LPS22HB_WriteRegister(uint8_t reg, uint8_t value)
{
  return busWriteByte(deviceAddress, reg, value);
}

uint8_t LPS22HB_Init(void)
{
  if((busReadByte == 0) || (busReadBytes == 0) || (busWriteByte == 0) || (busWriteBytes == 0))
  {
    return 0;
  }

  if(LPS22HB_IsPresent(LPS22HB_ADDRESS))
  {
    deviceAddress = LPS22HB_ADDRESS;
  }
  else if(LPS22HB_IsPresent(LPS22HB_ADDRESS_ALT))
  {
    deviceAddress = LPS22HB_ADDRESS_ALT;
  }
  else
  {
    return 0;
  }

  if(!LPS22HB_WriteRegister(LPS22HB_CTRL_REG2, LPS22HB_CTRL_REG2_CONFIG))
  {
    return 0;
  }

  if(!LPS22HB_WriteRegister(LPS22HB_CTRL_REG1, LPS22HB_CTRL_REG1_CONFIG))
  {
    return 0;
  }

  return 1;
}

uint8_t LPS22HB_ReadPressure(float *pressure)
{
  uint8_t raw[3];

  if((busReadBytes == 0) || (pressure == 0))
  {
    return 0;
  }

  if(!busReadBytes(deviceAddress, LPS22HB_PRESS_OUT_XL, raw, 3))
  {
    return 0;
  }

  int32_t value = ((int32_t)raw[2] << 16) | ((int32_t)raw[1] << 8) | (int32_t)raw[0];

  if((value & 0x00800000) != 0)
  {
    value |= (int32_t)0xFF000000;
  }

  *pressure = (float)value / LPS22HB_PRESSURE_LSB;

  return 1;
}

uint8_t LPS22HB_ReadTemperature(float *temperature)
{
  uint8_t raw[2];

  if((busReadBytes == 0) || (temperature == 0))
  {
    return 0;
  }

  if(!busReadBytes(deviceAddress, LPS22HB_TEMP_OUT_L, raw, 2))
  {
    return 0;
  }

  int16_t value = (int16_t)(((uint16_t)raw[1] << 8) | (uint16_t)raw[0]);

  *temperature = (float)value / LPS22HB_TEMPERATURE_LSB;

  return 1;
}

uint8_t LPS22HB_SetGroundReference(void)
{
  float pressure = 0.0f;

  if(!LPS22HB_ReadPressure(&pressure))
  {
    return 0;
  }

  if(pressure <= 0.0f)
  {
    return 0;
  }

  groundPressure = pressure;

  return 1;
}

float LPS22HB_CalculateAltitude(float pressure)
{
  if((pressure <= 0.0f) || (groundPressure <= 0.0f))
  {
    return 0.0f;
  }

  return LPS22HB_ALTITUDE_FACTOR *
         (1.0f - powf(pressure / groundPressure, LPS22HB_ALTITUDE_EXPONENT));
}
