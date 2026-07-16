/*
 * can.c
 *
 * Module B STM32
 * Bare-metal CAN
 *
 * PD0 : CAN1_RX
 * PD1 : CAN1_TX
 * CAN Bitrate : 500 kbps
 */

#include "can.h"


#define MODULE_ANNOUNCE_PERIOD_MS      100U
#define TEMPERATURE_SEND_PERIOD_MS     100U


/* ================= 내부 상태 ================= */

static volatile ModuleLinkState_t module_link_state =
    eMODULE_LINK_WAIT;

/* Base에서 받은 냉각 명령 */
static volatile uint8_t peltier_duty = 0U;

static uint32_t last_announce_tick = 0U;
static uint32_t last_temperature_tick = 0U;

static volatile int16_t target_temp_x10 = 0;
static volatile uint8_t target_temp_updated = 0U;
/* ================= 내부 함수 선언 ================= */

static void CAN1_GPIO_Init(void);
static void CAN1_Peripheral_Init(void);
static void CAN1_Filter_Init(void);
static void CAN1_RX_Interrupt_Init(void);

static uint8_t CAN1_TrySend(const CAN_Frame_t *frame);

static void CAN_ModuleB_SendAnnounce(void);
static void CAN_ModuleB_SendTemperature(int16_t temperature_x10);

static void CAN_ModuleB_HandleLinkResult(const CAN_Frame_t *frame);
static void CAN_ModuleB_HandleDutyCommand(const CAN_Frame_t *frame);
static void CAN_ModuleB_HandleTargetTempCommand(const CAN_Frame_t *frame);


/* ================= CAN 초기화 ================= */

void CAN_Init(void)
{
    CAN1_GPIO_Init();
    CAN1_Peripheral_Init();
    CAN1_Filter_Init();
    CAN1_RX_Interrupt_Init();

    module_link_state = eMODULE_LINK_WAIT;

    peltier_duty = 0U;
    target_temp_x10 = 0;
    target_temp_updated = 0U;

    last_announce_tick = HAL_GetTick();
    last_temperature_tick = HAL_GetTick();
}

/* ================= GPIO / CAN Register ================= */

static void CAN1_GPIO_Init(void)
{
    RCC->AHB1ENR |= (1U << 3);

    /* PD0, PD1 = Alternate Function */
    GPIOD->MODER &= ~((3U << 0) | (3U << 2));
    GPIOD->MODER |=  ((2U << 0) | (2U << 2));

    /* No Pull */
    GPIOD->PUPDR &= ~((3U << 0) | (3U << 2));

    /* High Speed */
    GPIOD->OSPEEDR |= ((3U << 0) | (3U << 2));

    /* PD0, PD1 = AF9 CAN1 */
    GPIOD->AFR[0] &= ~((0xFU << 0) | (0xFU << 4));
    GPIOD->AFR[0] |=  ((9U << 0) | (9U << 4));
}


static void CAN1_Peripheral_Init(void)
{
    RCC->APB1ENR |= (1U << 25);

    /* Sleep mode 해제 */
    CAN1->MCR &= ~(1U << 1);

    while (CAN1->MSR & (1U << 1))
    {
    }

    /* Init mode 진입 */
    CAN1->MCR |= (1U << 0);

    while ((CAN1->MSR & (1U << 0)) == 0U)
    {
    }

    /*
     * PCLK1 = 42MHz
     * Prescaler = 6
     * BS1 = 11 tq
     * BS2 = 2 tq
     * 500 kbps
     */
#if 0
    CAN1->BTR =
        (5U  << 0)  |
        (10U << 16) |
        (1U  << 20) |
        (0U  << 24);
#else
    CAN1->BTR =
        (11U  << 0)  |
        (10U << 16) |
        (1U  << 20) |
        (0U  << 24);
#endif
    /* 자동 재전송 사용 */
    CAN1->MCR &= ~((1U << 7) |
                   (1U << 6) |
                   (1U << 5) |
                   (1U << 4) |
                   (1U << 3) |
                   (1U << 2));

    /* Normal mode */
    CAN1->MCR &= ~(1U << 0);

    while (CAN1->MSR & (1U << 0))
    {
    }
}


static void CAN1_Filter_Init(void)
{
    CAN1->FMR |= (1U << 0);

    CAN1->FA1R &= ~(1U << 0);

    CAN1->FM1R &= ~(1U << 0);

    CAN1->FS1R |= (1U << 0);

    CAN1->FFA1R &= ~(1U << 0);

    /* 모든 CAN ID 수신 허용 */
    CAN1->sFilterRegister[0].FR1 = 0x00000000U;
    CAN1->sFilterRegister[0].FR2 = 0x00000000U;

    CAN1->FA1R |= (1U << 0);

    CAN1->FMR &= ~(1U << 0);
}


static void CAN1_RX_Interrupt_Init(void)
{
    CAN1->IER |= (1U << 1);

    NVIC_SetPriority(CAN1_RX0_IRQn, 5);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
}


/* ================= CAN 송신 ================= */

uint8_t CAN_Send(const CAN_Frame_t *frame)
{
    if ((frame == NULL) ||
        (frame->dlc > 8U))
    {
        return 0U;
    }

    return CAN1_TrySend(frame);
}


static uint8_t CAN1_TrySend(const CAN_Frame_t *frame)
{
    uint32_t mailbox;
    uint32_t tdlr;
    uint32_t tdhr;

    if (CAN1->TSR & (1U << 26))
    {
        mailbox = 0U;
    }
    else if (CAN1->TSR & (1U << 27))
    {
        mailbox = 1U;
    }
    else if (CAN1->TSR & (1U << 28))
    {
        mailbox = 2U;
    }
    else
    {
        return 0U;
    }

    tdlr =
        ((uint32_t)frame->data[0] << 0)  |
        ((uint32_t)frame->data[1] << 8)  |
        ((uint32_t)frame->data[2] << 16) |
        ((uint32_t)frame->data[3] << 24);

    tdhr =
        ((uint32_t)frame->data[4] << 0)  |
        ((uint32_t)frame->data[5] << 8)  |
        ((uint32_t)frame->data[6] << 16) |
        ((uint32_t)frame->data[7] << 24);

    CAN1->sTxMailBox[mailbox].TDTR = frame->dlc;
    CAN1->sTxMailBox[mailbox].TDLR = tdlr;
    CAN1->sTxMailBox[mailbox].TDHR = tdhr;

    CAN1->sTxMailBox[mailbox].TIR =
        ((uint32_t)(frame->std_id & 0x7FFU) << 21) |
        (1U << 0);

    return 1U;
}


/* ================= CAN 수신 ================= */

void CAN_RxIrqHandler(void)
{
    CAN_Frame_t rx_frame;

    while ((CAN1->RF0R & 0x3U) != 0U)
    {
        uint32_t rir;
        uint32_t rdtr;
        uint32_t rdlr;
        uint32_t rdhr;

        rir  = CAN1->sFIFOMailBox[0].RIR;
        rdtr = CAN1->sFIFOMailBox[0].RDTR;
        rdlr = CAN1->sFIFOMailBox[0].RDLR;
        rdhr = CAN1->sFIFOMailBox[0].RDHR;

        /* FIFO0 해제 */
        CAN1->RF0R |= (1U << 5);

        /* Extended / Remote Frame 무시 */
        if ((rir & (1U << 2)) ||
            (rir & (1U << 1)))
        {
            continue;
        }

        rx_frame.std_id =
            (uint16_t)((rir >> 21) & 0x7FFU);

        rx_frame.dlc =
            (uint8_t)(rdtr & 0x0FU);

        if (rx_frame.dlc > 8U)
        {
            rx_frame.dlc = 8U;
        }

        rx_frame.data[0] = (uint8_t)(rdlr & 0xFFU);
        rx_frame.data[1] = (uint8_t)((rdlr >> 8) & 0xFFU);
        rx_frame.data[2] = (uint8_t)((rdlr >> 16) & 0xFFU);
        rx_frame.data[3] = (uint8_t)((rdlr >> 24) & 0xFFU);

        rx_frame.data[4] = (uint8_t)(rdhr & 0xFFU);
        rx_frame.data[5] = (uint8_t)((rdhr >> 8) & 0xFFU);
        rx_frame.data[6] = (uint8_t)((rdhr >> 16) & 0xFFU);
        rx_frame.data[7] = (uint8_t)((rdhr >> 24) & 0xFFU);

        if (rx_frame.std_id == CAN_ID_LINK_RESULT)
        {
            CAN_ModuleB_HandleLinkResult(&rx_frame);
        }
        else if (rx_frame.std_id == CAN_ID_COLD_DUTY_COMMAND)
        {
            CAN_ModuleB_HandleDutyCommand(&rx_frame);
        }
        else if (rx_frame.std_id == CAN_ID_COLD_TARGET_TEMP)
        {
            CAN_ModuleB_HandleTargetTempCommand(&rx_frame);
        }
    }
}


/* ================= Module B Protocol ================= */

void CAN_ModuleB_Process(int16_t temperature_x10)
{
    uint32_t now_tick;

    now_tick = HAL_GetTick();

    /* 인증 전: ID 주기 전송 */
    if (module_link_state == eMODULE_LINK_WAIT)
    {
        if ((now_tick - last_announce_tick) >=
            MODULE_ANNOUNCE_PERIOD_MS)
        {
            last_announce_tick = now_tick;

            CAN_ModuleB_SendAnnounce();
        }
    }

    /* 승인 후: 현재 온도 주기 전송 */
    else if (module_link_state == eMODULE_LINK_ACCEPTED)
    {
        if ((now_tick - last_temperature_tick) >=
            TEMPERATURE_SEND_PERIOD_MS)
        {
            last_temperature_tick = now_tick;

            CAN_ModuleB_SendTemperature(temperature_x10);
        }
    }
}


/* 0x110 : Module B ID 전송 */
static void CAN_ModuleB_SendAnnounce(void)
{
    CAN_Frame_t frame = {0};

    frame.std_id = CAN_ID_MODULE_ANNOUNCE;
    frame.dlc = 5U;

    frame.data[0] = (uint8_t)eMODULE_COLD_CHAIN;

    frame.data[1] = (uint8_t)(MODULE_B_ID & 0xFFU);
    frame.data[2] = (uint8_t)((MODULE_B_ID >> 8) & 0xFFU);
    frame.data[3] = (uint8_t)((MODULE_B_ID >> 16) & 0xFFU);
    frame.data[4] = (uint8_t)((MODULE_B_ID >> 24) & 0xFFU);

    (void)CAN_Send(&frame);
}


/* 0x173 : 현재 온도 전송 */
static void CAN_ModuleB_SendTemperature(int16_t temperature_x10)
{
    CAN_Frame_t frame = {0};
    uint16_t raw_temperature;

    raw_temperature = (uint16_t)temperature_x10;

    frame.std_id = CAN_ID_COLD_TEMPERATURE;
    frame.dlc = 2U;

    frame.data[0] = (uint8_t)(raw_temperature & 0xFFU);
    frame.data[1] = (uint8_t)((raw_temperature >> 8) & 0xFFU);

    (void)CAN_Send(&frame);
}


/* 0x140 : 인증 결과 처리 */
static void CAN_ModuleB_HandleLinkResult(const CAN_Frame_t *frame)
{
    if (frame->dlc < 3U)
    {
        return;
    }

    if (frame->data[0] != (uint8_t)eMODULE_COLD_CHAIN)
    {
        return;
    }

    if (frame->data[1] == 1U)
    {
        module_link_state = eMODULE_LINK_ACCEPTED;
    }
    else
    {
        module_link_state = eMODULE_LINK_REJECTED;

        peltier_duty = 0U;
        target_temp_x10 = 0;
    }
}


/* 0x172 : 펠티어 Duty 수신 */
static void CAN_ModuleB_HandleDutyCommand(const CAN_Frame_t *frame)
{
    uint8_t received_duty;

    if (frame->dlc < 1U)
    {
        return;
    }

    if (module_link_state != eMODULE_LINK_ACCEPTED)
    {
        return;
    }

    received_duty = frame->data[0];

    if (received_duty > 100U)
    {
        received_duty = 100U;
    }

    peltier_duty = received_duty;
}


/* 0x174 : 목표 온도 수신 */
static void CAN_ModuleB_HandleTargetTempCommand(const CAN_Frame_t *frame)
{
    if (frame->dlc < 2U)
    {
        return;
    }

    if (module_link_state != eMODULE_LINK_ACCEPTED)
    {
        return;
    }

    target_temp_x10 =
        (int16_t)(
            ((uint16_t)frame->data[0] << 0) |
            ((uint16_t)frame->data[1] << 8)
        );

    /* 새 목표 온도 수신 표시 */
    target_temp_updated = 1U;
}


/* ================= 상태 조회 ================= */

uint8_t CAN_ModuleB_GetPeltierDuty(void)
{
    return peltier_duty;
}


int16_t CAN_ModuleB_GetTargetTempX10(void)
{
    return target_temp_x10;
}


ModuleLinkState_t CAN_ModuleB_GetLinkState(void)
{
    return module_link_state;
}


uint8_t CAN_ModuleB_IsAccepted(void)
{
    if (module_link_state == eMODULE_LINK_ACCEPTED)
    {
        return 1U;
    }

    return 0U;
}

uint8_t CAN_ModuleB_TakeTargetTempX10(int16_t *target_temp_value)
{
    if (target_temp_value == NULL)
    {
        return 0U;
    }

    if (target_temp_updated == 0U)
    {
        return 0U;
    }

    *target_temp_value = target_temp_x10;

    /* main에서 한 번 읽었으므로 다시 0 */
    target_temp_updated = 0U;

    return 1U;
}
