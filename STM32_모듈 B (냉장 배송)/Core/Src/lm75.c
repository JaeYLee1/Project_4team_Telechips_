/*
 * lm75.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "lm75.h"

static I2C_HandleTypeDef *lm75_hi2c = NULL;

static int16_t LM75_ReadRaw(uint16_t addr);
static float LM75_ConvertTemp(int16_t raw);

void LM75_Init(I2C_HandleTypeDef *hi2c)
{
    lm75_hi2c = hi2c;
}

HAL_StatusTypeDef LM75_ReadTemperature(
        uint16_t addr,
        float *temperature)
{
    int16_t raw;

    if(lm75_hi2c == NULL)
        return HAL_ERROR;

    raw = LM75_ReadRaw(addr);

    *temperature = LM75_ConvertTemp(raw);

    return HAL_OK;
}

static int16_t LM75_ReadRaw(uint16_t addr)
{
    uint8_t data[2];

    if(HAL_I2C_Mem_Read(
            lm75_hi2c,
            addr,
            TEMP_REG,
            I2C_MEMADD_SIZE_8BIT,
            data,
            2,
            HAL_MAX_DELAY) != HAL_OK)
    {
        return 0;
    }

    int16_t raw;

    raw = (data[0] << 8) | data[1];
//    raw >>= 7;
    raw >>= 5;  // 11비트 데이터

    return raw;
}

static float LM75_ConvertTemp(int16_t raw)
{
//    return raw * 0.5f;
	return raw * 0.125f;  // 11비트 데이터
}
