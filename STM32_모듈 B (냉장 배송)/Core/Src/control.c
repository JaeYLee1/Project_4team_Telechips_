/*
 * control.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "shared.h"
#include "control.h"
#include "motor.h"

// limits
#define HOT_LIMIT   50.0f
#define HOT_WARN    45.0f

void Control_Task(void)
{
    float cold   = sys_data.temp.cold;
    float hot    = sys_data.temp.hot;
    float target = sys_data.input.target_temp;

    uint16_t *peltier = &sys_data.output.peltier;
    uint16_t *fan     = &sys_data.output.fan;

    // 1. 센서 유효성 검사 & 2. 과열 보호
    if ((sys_data.temp.valid_flag == 0) || (hot >= HOT_LIMIT))
    {
        sys_data.state = STATE_FAULT;
        *peltier = 0;  // 릴레이 상태 로그용 (0: OFF)
        *fan = 1000;

        Peltier_Off();  // 릴레이 물리적 차단 (PB8 = Low)
        Fan_SetDuty(*fan);
        Peltier_SetDuty(*peltier);  // 1초 주기로 상태 로그 출력
        return;
    }

    // 3. 제어 로직 (히스테리시스 구간 반영 온오프 제어)
    // 현재 온도가 목표 온도 + 3.0도보다 높아지면 냉각 가동 시작
    if (cold > target + 3.0f)
    {
        sys_data.state = STATE_COOLING;
        *peltier = 1;  // 릴레이 상태 로그용 (1: ON)

        Peltier_On();  // 릴레이 물리적 가동 (PB8 = High)
        *fan = 1000;
    }
    // 현재 온도가 목표 온도 이하로 확실히 떨어지면 냉각 중지
    // (target ~ target + 3.0f 구간에서는 이전 상태를 유지하여 릴레이 Chattering 방지)
    else if (cold <= target)
    {
        sys_data.state = STATE_HOLD;
        *peltier = 0;  // 릴레이 상태 로그용 (0: OFF)

        Peltier_Off();  // 릴레이 물리적 차단 (PB8 = Low)
        *fan = 300;
    }

    // Hot side 경고 온도를 넘었을 경우 팬은 무조건 최대 속도로 방열
    if (hot >= HOT_WARN)
    {
        *fan = 1000;
    }

    // 4. 최종 출력 레이어 전송
    Fan_SetDuty(*fan);
    Peltier_SetDuty(*peltier);  // motor.c 내부의 Uart3_Printf 로그 출력 연결
}
