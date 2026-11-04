#include "hts221.h"

static hts221_read_byte_t busReadByte = 0;
static hts221_read_bytes_t busReadBytes = 0;
static hts221_write_byte_t busWriteByte = 0;
static hts221_write_bytes_t busWriteBytes = 0;

static float humidity0 = 0.0f;
static float humidity1 = 0.0f;
static float temperature0 = 0.0f;
static float temperature1 = 0.0f;

static int16_t humidity0Raw = 0;
static int16_t humidity1Raw = 0;
static int16_t temperature0Raw = 0;
static int16_t temperature1Raw = 0;

void HTS221_RegisterBus(hts221_read_byte_t readByte, hts221_read_bytes_t readBytes,
                        hts221_write_byte_t writeByte, hts221_write_bytes_t writeBytes)
{
  busReadByte = readByte;
  busReadBytes = readBytes;
  busWriteByte = writeByte;
  busWriteBytes = writeBytes;
}

static int16_t HTS221_ToInt16(uint8_t low, uint8_t high)
{
  return (int16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

static uint8_t HTS221_WriteRegister(uint8_t reg, uint8_t value)
{
  return busWriteByte(HTS221_ADDRESS, reg, value);
}

static uint8_t HTS221_ReadCalibration(void)
{
  uint8_t calibration[HTS221_CALIBRATION_SIZE];

  if(!busReadBytes(HTS221_ADDRESS, HTS221_CALIBRATION_START | HTS221_AUTO_INCREMENT,
                   calibration, HTS221_CALIBRATION_SIZE))
  {
    return 0;
  }

  humidity0 = (float)calibration[0] / 2.0f;
  humidity1 = (float)calibration[1] / 2.0f;

  uint16_t rawTemperature0 = (uint16_t)(((uint16_t)(calibration[5] & 0x03) << 8) | calibration[2]);
  uint16_t rawTemperature1 = (uint16_t)(((uint16_t)(calibration[5] & 0x0C) << 6) | calibration[3]);

  temperature0 = (float)rawTemperature0 / 8.0f;
  temperature1 = (float)rawTemperature1 / 8.0f;

  humidity0Raw = HTS221_ToInt16(calibration[6], calibration[7]);
  humidity1Raw = HTS221_ToInt16(calibration[10], calibration[11]);
  temperature0Raw = HTS221_ToInt16(calibration[12], calibration[13]);
  temperature1Raw = HTS221_ToInt16(calibration[14], calibration[15]);

  return 1;
}

uint8_t HTS221_Init(void)
{
  uint8_t id = 0;

  if((busReadByte == 0) || (busReadBytes == 0) || (busWriteByte == 0) || (busWriteBytes == 0))
  {
    return 0;
  }

  if(!busReadByte(HTS221_ADDRESS, HTS221_WHO_AM_I, &id))
  {
    return 0;
  }

  if(id != HTS221_WHO_AM_I_VALUE)
  {
    return 0;
  }

  if(!HTS221_WriteRegister(HTS221_AV_CONF, HTS221_AV_CONF_CONFIG))
  {
    return 0;
  }

  if(!HTS221_WriteRegister(HTS221_CTRL_REG1, HTS221_CTRL_REG1_CONFIG))
  {
    return 0;
  }

  return HTS221_ReadCalibration();
}

float HTS221_Interpolate(int16_t raw, int16_t raw0, int16_t raw1, float value0, float value1)
{
  if(raw1 == raw0)
  {
    return value0;
  }

  return value0 + (((float)(raw - raw0) * (value1 - value0)) / (float)(raw1 - raw0));
}

uint8_t HTS221_ReadHumidity(float *humidity)
{
  uint8_t raw[2];

  if((busReadBytes == 0) || (humidity == 0))
  {
    return 0;
  }

  if(!busReadBytes(HTS221_ADDRESS, HTS221_HUMIDITY_OUT_L | HTS221_AUTO_INCREMENT, raw, 2))
  {
    return 0;
  }

  float value = HTS221_Interpolate(HTS221_ToInt16(raw[0], raw[1]),
                                   humidity0Raw, humidity1Raw, humidity0, humidity1);

  if(value < 0.0f)
  {
    value = 0.0f;
  }

  if(value > 100.0f)
  {
    value = 100.0f;
  }

  *humidity = value;

  return 1;
}

uint8_t HTS221_ReadTemperature(float *temperature)
{
  uint8_t raw[2];

  if((busReadBytes == 0) || (temperature == 0))
  {
    return 0;
  }

  if(!busReadBytes(HTS221_ADDRESS, HTS221_TEMP_OUT_L | HTS221_AUTO_INCREMENT, raw, 2))
  {
    return 0;
  }

  *temperature = HTS221_Interpolate(HTS221_ToInt16(raw[0], raw[1]),
                                    temperature0Raw, temperature1Raw,
                                    temperature0, temperature1);

  return 1;
}
