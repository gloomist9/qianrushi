#include "motor_serial.h"
#include "string.h"
#include "stdio.h"

/* =========================
   基础定义
   ========================= */

#define MOTOR_RX_BUF_SIZE 128
#define MOTOR_FRAME_MAX   128

/* =========================
   接收缓冲
   ========================= */

uint8_t motor_dma_buf[MOTOR_RX_BUF_SIZE];
uint8_t motor_frame_buf[MOTOR_FRAME_MAX];

/* =========================
   电机对象
   ========================= */

MotorInfo motor1 = {0};
MotorInfo motor2 = {0};

/* =========================
   获取电机
   ========================= */

MotorInfo* get_motor_by_id(uint8_t id)
{
    if(id == 1) return &motor1;
    if(id == 2) return &motor2;
    return NULL;
}

/* =========================
   初始化USART1 DMA + IDLE
   ========================= */

void motor_serial_init(void)
{
    memset(&motor1, 0, sizeof(MotorInfo));
    memset(&motor2, 0, sizeof(MotorInfo));

    motor1.id = 1;
    motor2.id = 2;

    HAL_UART_Receive_DMA(&huart1, motor_dma_buf, MOTOR_RX_BUF_SIZE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
}

/* =========================
   USART1中断入口
   ========================= */

void USART1_IRQHandler(void)
{
    /* IDLE中断 */
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        HAL_UART_DMAStop(&huart1);

        uint16_t len = MOTOR_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);

        extern volatile uint32_t dbg_usart1_idle, dbg_usart1_len;
        dbg_usart1_idle++;
        dbg_usart1_len = len;

        if(len > 0 && len < MOTOR_FRAME_MAX)
        {
            memcpy(motor_frame_buf, motor_dma_buf, len);
            motor_frame_parse(motor_frame_buf, len);
        }

        HAL_UART_Receive_DMA(&huart1, motor_dma_buf, MOTOR_RX_BUF_SIZE);
    }

    HAL_UART_IRQHandler(&huart1);
}

/* =========================
   帧解析入口
   ========================= */

void motor_frame_parse(uint8_t *buf, uint16_t len)
{
    /*
     * 电机回复格式（Read_Sys_Params 查询 S_FLAG）：
     *   短帧 4 字节: {id, 0x03, status, 0x6B}
     *   长帧 7+ 字节: {id, 0x03, dlc[2], data..., 0x6B}
     * 最短帧 4 字节即可解析
     */
    if(len < 4) return;

    for(int i = 0; i <= len - 4; i++)
    {
        uint8_t *frame = &buf[i];
        uint8_t id   = frame[0];
        uint8_t func = frame[1];

        /* 电机回复功能码和查询一致（S_FLAG=0x3A），不固定为 0x03 */
        if(func < 0x20) continue;

        uint8_t status;
        uint8_t remain = (uint8_t)(len - i);

        if(remain >= 7)
        {
            /* 尝试长帧格式（带 DLC） */
            uint16_t dlc = (uint16_t)frame[2];
            if(dlc != 0x02)
                dlc = ((uint16_t)frame[2] << 8) | frame[3];
            if(dlc == 0x02)
                status = frame[4];
            else
            {
                /* DLC 不匹配，当短帧处理 */
                status = frame[2];
            }
        }
        else
        {
            /* 短帧: {id, 0x03, status, 0x6B}，frame[2] 就是状态 */
            status = frame[2];
        }

        extern volatile uint32_t dbg_frame_parse, dbg_idle_found, dbg_run_found;
        extern volatile uint32_t dbg_motor1_state, dbg_motor2_state;
        dbg_frame_parse++;

        MotorInfo *m = get_motor_by_id(id);
        if(m == NULL) continue;

        /* ===== 状态解析（核心）=====
         * S_FLAG 回复的 status 字节含义：
         *   0x00 = 待机/空闲
         *   0x01 = 正在运动
         *   0x02 = 位置到达（运动完成，即 IDLE）
         *   其他 = 错误状态
         */
        switch(status)
        {
            case 0x00:
            case 0x02:   /* 位置到达 = IDLE */
                m->state = MOTOR_IDLE;
                dbg_idle_found++;
                break;

            case 0x01:
                m->state = MOTOR_RUNNING;
                dbg_run_found++;
                break;

            default:
                m->state = MOTOR_ERROR;
                break;
        }

        m->online = 1;
        m->last_tick = HAL_GetTick();
        m->expect_reply = 0;

        /* 更新全局状态快照 */
        {
            extern MotorInfo motor1, motor2;
            extern volatile uint32_t dbg_motor1_state, dbg_motor2_state;
            dbg_motor1_state = motor1.state;
            dbg_motor2_state = motor2.state;
        }

        /* 找到一帧就够 */
        break;
    }
}

/* =========================
   busy判断（planner用）
   ========================= */

uint8_t motion_is_busy(void)
{
    if(motor1.state == MOTOR_RUNNING) return 1;
    if(motor2.state == MOTOR_RUNNING) return 1;

    return 0;
}

/* =========================
   超时监控
   ========================= */

#define MOTOR_TIMEOUT_MS 200

void motor_watchdog(void)
{
    uint32_t now = HAL_GetTick();

    MotorInfo *list[2] = {&motor1, &motor2};

    for(int i = 0; i < 2; i++)
    {
        MotorInfo *m = list[i];

        if(m->last_tick == 0)
            continue;

        if(now - m->last_tick > MOTOR_TIMEOUT_MS)
        {
            m->state = MOTOR_TIMEOUT;
            m->online = 0;
        }
    }
}

