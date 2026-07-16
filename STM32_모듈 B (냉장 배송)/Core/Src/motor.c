/*
 * motor.c
 *
 *  Created on: 2026. 7. 6.
 *      Author: 한국전파진흥협회
 */

#include "motor.h"
#include "uart3.h"

// GPIO: PB8
void Peltier_GPIO_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

	GPIOB->MODER &= ~(0x3 << (8 * 2));
	GPIOB->MODER |= (0x1 << (8 * 2)); // 01: Output mode

	GPIOB->OTYPER &= ~(0x1 << 8);  // push-pull
}

void Peltier_Init(void)
{
    Peltier_GPIO_Init();
    Peltier_Off(); // 안전을 위해 초기 상태 OFF
}

void Peltier_SetDuty(uint16_t duty)
{
    static uint32_t prev_tick = 0;

    if (HAL_GetTick() - prev_tick >= 1000)
    {
        prev_tick = HAL_GetTick();
        Uart3_Printf("[Peltier] Relay = %s\r\n", (duty > 0) ? "ON" : "OFF");
    }
}

// 릴레이 켬 (PB8 = High -> 3.3V 출력)
void Peltier_On(void)
{
    GPIOB->BSRR = (0x1 << 8);
}


// 릴레이 끔 (PB8 = Low -> 0V 출력)
void Peltier_Off(void)
{
    // BSRR 레지스터 상위 16비트 영역: Reset(Low) (8 + 16 = 24번 비트)
    GPIOB->BSRR = (0x1 << (8 + 16));
}

// GPIO: PB15
void Fan_GPIO_Init(void)
{
    // GPIOB clock enable
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // PB15 output mode
    GPIOB->MODER &= ~(0x3 << (15 * 2));
    GPIOB->MODER |= (0x1 << (15 * 2));

    // push-pull
    GPIOB->OTYPER &= ~(0x1 << 15);
}

// PWM GPIO: PA3 (TIM9_CH2)
void Fan_PWM_GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // PA3 AF mode
    GPIOA->MODER &= ~(0x3 << (3 * 2));
    GPIOA->MODER |= (0x2 << (3 * 2));

    // AF3 = TIM9
    GPIOA->AFR[0] &= ~(0xF << (3 * 4));
    GPIOA->AFR[0] |= (0x3 << (3 * 4));
}

// TIM9 PWM INIT (CH2)
void Fan_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;

    TIM9->CR1 &= ~TIM_CR1_CEN;

    TIM9->PSC = 84 - 1;    //
    TIM9->ARR = 1000 - 1;    // 0~1000 duty

    TIM9->CNT = 0;

    // CH2 PWM mode
    TIM9->CCMR1 &= ~(TIM_CCMR1_CC2S);
    TIM9->CCMR1 &= ~(0x7 << 12);
    TIM9->CCMR1 |= (0x6 << 12);  // PWM mode 1

    TIM9->CCMR1 |= TIM_CCMR1_OC2PE;

    // polarity
    TIM9->CCER &= ~TIM_CCER_CC2P;

    // enable output
    TIM9->CCER |= TIM_CCER_CC2E;

    TIM9->EGR |= TIM_EGR_UG;

    TIM9->CR1 |= TIM_CR1_CEN;
}

// INIT ALL
void Fan_Init(void)
{
	Fan_GPIO_Init();  // PB15
    Fan_PWM_GPIO_Init();
    Fan_PWM_Init();

    GPIOB->BSRR = (1 << (15 + 16));  // 방향 기본값 (PB15 = Low)
}

// SPEED CONTROL
void Fan_SetDuty(uint16_t duty)
{
	static uint32_t prev_tick = 0;

	if (duty > 1000)
		duty = 1000;

    TIM9->CCR2 = duty;

    if (duty > 0)
    {
    	GPIOB->BSRR = (1 << 15);        // PB15 = High (구동 허용)
    }
    else
    {
    	GPIOB->BSRR = (1 << (15 + 16)); // PB15 = Low (구동 차단)
    }

    if (HAL_GetTick() - prev_tick >= 1000)
    {
    	prev_tick = HAL_GetTick();

        Uart3_Printf("[Fan]     PWM = %3d%%\r\n", duty * 100 / 1000);
    }
}

// OPTIONAL CONTROL
void Fan_On(void)
{
    TIM9->CCR2 = 1000;  // Fan_SetDuty(1000);
	GPIOB->BSRR = (1 << 15);  // PB15 = High
}

void Fan_Off(void)
{
    TIM9->CCR2 = 0;  // Fan_SetDuty(0);
    GPIOB->BSRR = (1 << (15 + 16));  // PB15 = LOW
}
