/*
 * uart3.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "uart3.h"

#include <stdarg.h>
#include <stdio.h>


extern UART_HandleTypeDef huart3;

void Uart3_Send_Byte(char ch)
{
    HAL_UART_Transmit(
        &huart3,
        (uint8_t *)&ch,
        1,
        HAL_MAX_DELAY
    );
}

void Uart3_Send_String(char *str)
{
    while(*str)
    {
        Uart3_Send_Byte(*str++);
    }
}

void Uart3_Printf(char *fmt,...)
{
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    Uart3_Send_String(buffer);
}

