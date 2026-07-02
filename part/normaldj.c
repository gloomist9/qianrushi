#include "main.h"
#include "tim.h"
//平台电机
void platformmove(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 60);
    __HAL_TIM_SET_AUTORELOAD(&htim1, 3000);
}
void platformstop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
}
//笔电机
void penmove(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 60);
    __HAL_TIM_SET_AUTORELOAD(&htim1, 3000);
}
void penstop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

