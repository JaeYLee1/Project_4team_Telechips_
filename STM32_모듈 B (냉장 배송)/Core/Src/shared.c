/*
 * shared.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "shared.h"

// 공유 구조체 초기화
SystemData sys_data = {
    .temp = {
        .cold = 0.0f,
        .hot = 0.0f,
        .valid_flag = 1
    },
    .input = {
        .target_temp = 25.0f,  // 안전을 위한 초기 기본값 설정
        .peltier_en = 0  // 초기값: 펠티어 비활성화 (0)
    },
	.output = {
        .peltier = 0,
        .fan = 0
    },
    .state = STATE_INIT
};
