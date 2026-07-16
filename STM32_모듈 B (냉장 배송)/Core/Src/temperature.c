/*
 * temperature.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "shared.h"
#include "temperature.h"
#include "lm75.h"

// Filter memory (최초 진입 구분을 위해 -999.0f로 초기화)
static float cold_prev = -999.0f;
static float hot_prev = -999.0f;

// Low-pass filter
static float Filter(float old, float new)
{
    return 0.8f * old + 0.2f * new;
}

void Temperature_Task(void)
{
    float t_raw = 0.0f;
    uint8_t valid = 1;

    float *cold = &sys_data.temp.cold;
    float *hot  = &sys_data.temp.hot;

    // 1. Cold sensor 읽기
    // LM75_ReadTemperature 호출, 리턴값 HAL_OK인지 확인
    if (LM75_ReadTemperature(LM75_COLD_ADDR, &t_raw) != HAL_OK)
    {
        valid = 0;
        t_raw = cold_prev;  // 통신 에러 -> 직전 온도 강제 유지 혹은 필터 건너뜀
    }
    else
    {
        // 최초 구동 시 필터 지연 방지
        if (cold_prev == -999.0f) {
            cold_prev = t_raw;
        }

        *cold = Filter(cold_prev, t_raw);
        cold_prev = *cold;
    }

    t_raw = 0.0f;

    // 2. Hot sensor 읽기
    if (LM75_ReadTemperature(LM75_HOT_ADDR, &t_raw) != HAL_OK)
    {
        valid = 0;
    }
    else
    {
        // 최초 구동 시 필터 지연 방지
        if (hot_prev == -999.0f) {
            hot_prev = t_raw;
        }

        *hot = Filter(hot_prev, t_raw);
        hot_prev = *hot;
    }

    // 3. 소프트웨어 범위 검증
    if (*cold < -40.0f || *cold > 100.0f ||
    		*hot  < -40.0f || *hot  > 120.0f)
    {
        valid = 0;
    }

    // 최종 유효성 플래그 갱신
    sys_data.temp.valid_flag = valid;
}
