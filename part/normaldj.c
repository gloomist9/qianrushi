#include "main.h"
#include "tim.h"
#include "normaldj.h"

extern volatile uint16_t pulse_remaining;//main.c定义

//启动PWM发脉冲，发完自动停（软件计数，不限RCR的255上限）
void motor_pulse_start(uint16_t pulse_count)
{
    if (pulse_count == 0) return;

    //先停掉上次的
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_MOE_DISABLE(&htim1);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);
    __HAL_TIM_SET_AUTORELOAD(&htim1, 200);

    //RCR=0，每个PWM周期触发更新中断，软件计数
    htim1.Instance->RCR = 0;

    pulse_remaining = pulse_count;

    htim1.Instance->EGR |= TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_MOE_ENABLE(&htim1);
}

//抬笔
void penrise(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    motor_pulse_start(45);
}

//落笔
void pendown(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    motor_pulse_start(45);
}

void penstop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}
