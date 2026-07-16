/*
 * shared.h
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#ifndef INC_SHARED_H_
#define INC_SHARED_H_

#include <stdint.h>

// 시스템 상태 정의
typedef enum {
    STATE_INIT = 0,
    STATE_COOLING,
    STATE_HOLD,
    STATE_FAULT
} ControlState;

// 1. 센서 데이터 그룹
typedef struct {
    float cold;
    float hot;
    uint8_t valid_flag;
} TemperatureData;

// 2. 외부 입력 설정값 그룹
typedef struct {
    float target_temp;  // 목표 온도
    uint16_t peltier_en;  // 외부에서 지정한 펠티어 활성화 여부 (1: Enable, 0: Disable)
} ControlInputData;

// 3. 최종 하드웨어 출력 제어값 그룹
typedef struct {
    uint16_t peltier;       // Peltier 릴레이 상태 (1: ON, 0: OFF)
    uint16_t fan;           // Fan PWM 출력 (0~1000 스케일)
} HardwareOutputData;

// 4. 최상위 통합 구조체
typedef struct {
    TemperatureData temp;
    ControlInputData input;
    HardwareOutputData output;
    ControlState state;
} SystemData;

// 전역 공유 변수 선언
extern SystemData sys_data;

#endif /* INC_SHARED_H_ */
