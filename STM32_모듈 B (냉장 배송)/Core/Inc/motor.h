/*
 * motor.h
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "stm32f4xx_hal.h"

void Peltier_GPIO_Init(void);
void Peltier_Init(void);

void Peltier_SetDuty(uint16_t duty);
void Peltier_On(void);
void Peltier_Off(void);

void Fan_GPIO_Init(void);
void Fan_PWM_GPIO_Init(void);
void Fan_PWM_Init(void);
void Fan_Init(void);

void Fan_SetDuty(uint16_t duty);   // 0~1000
void Fan_On(void);
void Fan_Off(void);

#endif /* INC_MOTOR_H_ */

