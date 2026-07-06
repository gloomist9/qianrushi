#include "main.h"
#include "tim.h"
#include "normaldj.h"

/* main.c 定义的软件脉冲计数器，回调里递减 */
extern volatile uint16_t pulse_remaining;

/**
 * @brief  启动电机：发送指定数量的 PWM 脉冲后自动停止
 * @param  pulse_count  脉冲个数（软件计数，不限 RCR 的 8bit/255 限制）
 * @note   可多次调用，每次调用会重新启动 PWM，发完自动停
 */
void motor_pulse_start(uint16_t pulse_count)
{
    if (pulse_count == 0) return;

    /* 先停掉上一次的（如果还在跑），防止冲突 */
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_MOE_DISABLE(&htim1);

    /* ① 设置 PWM 参数 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);
    __HAL_TIM_SET_AUTORELOAD(&htim1, 200);

    /* ② RCR=0：每个 PWM 周期触发一次更新中断，由软件计数 */
    htim1.Instance->RCR = 0;

    /* ③ 存储目标脉冲数（回调里每次 --pulse_remaining，减到 0 停） */
    pulse_remaining = pulse_count;

    /* ④ 生成 UG 事件：预装载值 → 影子寄存器 */
    htim1.Instance->EGR |= TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    /* ⑤ 使能更新中断 */
    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

    /* ⑥ 启动 PWM + MOE */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_MOE_ENABLE(&htim1);
}

//抬笔控制
void penrise(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    motor_pulse_start(70);
}

void pendown(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    motor_pulse_start(70);
}

void penstop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

