/*
 * uart3.h
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#ifndef INC_UART3_H_
#define INC_UART3_H_

#include "main.h"

void Uart3_Send_Byte(char ch);
void Uart3_Send_String(char *str);
void Uart3_Printf(char *fmt, ...);

#endif /* INC_UART3_H_ */
