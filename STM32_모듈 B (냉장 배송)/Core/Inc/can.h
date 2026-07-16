/*
 * can.h
 *
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdint.h>

#include "main.h"


/* ================= CAN ID ================= */

/* 인증 */
#define CAN_ID_MODULE_ANNOUNCE       0x110U
#define CAN_ID_LINK_RESULT           0x140U

/* Module B */
#define CAN_ID_COLD_DUTY_COMMAND     0x172U
#define CAN_ID_COLD_TEMPERATURE      0x173U
#define CAN_ID_COLD_TARGET_TEMP      0x174U


/* ================= Module 정보 ================= */

typedef enum
{
    eMODULE_NONE       = 0U,
    eMODULE_GENERAL    = 1U,
    eMODULE_COLD_CHAIN = 2U,
    eMODULE_UNKNOWN    = 0xFFU
} ModuleType_t;

#define MODULE_B_ID                  0x0000B001U


/* ================= 연결 상태 ================= */

typedef enum
{
    eMODULE_LINK_WAIT = 0U,
    eMODULE_LINK_ACCEPTED,
    eMODULE_LINK_REJECTED
} ModuleLinkState_t;


/* ================= CAN Frame ================= */

typedef struct
{
    uint16_t std_id;
    uint8_t  dlc;
    uint8_t  data[8];
} CAN_Frame_t;


/* ================= CAN Driver ================= */

void CAN_Init(void);
void CAN_RxIrqHandler(void);

uint8_t CAN_Send(const CAN_Frame_t *frame);


/* ================= Module B Protocol ================= */

/*
 * main while문에서 반복 호출
 *
 * temperature_x10:
 * 25.3°C -> 253
 * -5.0°C -> -50
 */
void CAN_ModuleB_Process(int16_t temperature_x10);

/* Base가 보낸 펠티어 Duty */
uint8_t CAN_ModuleB_GetPeltierDuty(void);

/* Base가 보낸 목표 온도 */
int16_t CAN_ModuleB_GetTargetTempX10(void);

ModuleLinkState_t CAN_ModuleB_GetLinkState(void);

uint8_t CAN_ModuleB_IsAccepted(void);

uint8_t CAN_ModuleB_TakeTargetTempX10(int16_t *target_temp_x10);

#endif /* INC_CAN_H_ */
