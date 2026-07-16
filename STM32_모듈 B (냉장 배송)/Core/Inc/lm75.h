/*
 * lm75.h
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#ifndef INC_LM75_H_
#define INC_LM75_H_

#include "main.h"

#define LM75_COLD_ADDR (0x48 << 1)
#define LM75_HOT_ADDR (0x4F << 1)
#define TEMP_REG   0x00

void LM75_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef LM75_ReadTemperature(
        uint16_t addr,
        float *temperature);

#endif /* INC_LM75_H_ */
