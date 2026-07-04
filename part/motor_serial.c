#include "motor_serial.h"
#include "string.h"
#include "stdio.h"

/* =========================
   ��������
   ========================= */

#define MOTOR_RX_BUF_SIZE 128
#define MOTOR_FRAME_MAX   128

/* =========================
   ���ջ���
   ========================= */

uint8_t motor_dma_buf[MOTOR_RX_BUF_SIZE];
uint8_t motor_frame_buf[MOTOR_FRAME_MAX];

/* =========================
   �������
   ========================= */

MotorInfo motor1 = {0};
MotorInfo motor2 = {0};

/* =========================
   ��ȡ���
   ========================= */

MotorInfo* get_motor_by_id(uint8_t id)
{
    if(id == 1) return &motor1;
    if(id == 2) return &motor2;
    return NULL;
}

/* =========================
   ��ʼ��USART1 DMA + IDLE
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
   USART1�ж����
   ========================= */

void USART1_IRQHandler(void)
{
    /* IDLE�ж� */
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        HAL_UART_DMAStop(&huart1);

        uint16_t len = MOTOR_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);

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
   ֡�������
   ========================= */

void motor_frame_parse(uint8_t *buf, uint16_t len)
{
    if(len < 7) return;

    for(int i = 0; i <= len - 7; i++)
    {
        uint8_t *frame = &buf[i];

        /* ===== �̶�֡�ṹ��� ===== */
        uint8_t id     = frame[0];
        uint8_t func   = frame[1];
        uint16_t dlc   = (uint16_t)frame[2];
        if(dlc != 0x02)
        {
            dlc = ((uint16_t)frame[2] << 8) | frame[3];
        }
        uint8_t status = frame[4];

        if(func != 0x03) continue;
        if(dlc != 0x02) continue;

        MotorInfo *m = get_motor_by_id(id);
        if(m == NULL) continue;

        /* ===== ״̬���������ģ�===== */
        switch(status)
        {
            case 0x00:
                m->state = MOTOR_IDLE;
                break;

            case 0x01:
                m->state = MOTOR_RUNNING;
                break;

            case 0x02:
                m->state = MOTOR_ERROR;
                break;

            default:
                m->state = MOTOR_ERROR;
                break;
        }

        m->online = 1;
        m->last_tick = HAL_GetTick();
        m->expect_reply = 0;

        /* �ҵ�һ֡�͹� */
        break;
    }
}

/* =========================
   busy�жϣ�planner�ã�
   ========================= */

uint8_t motion_is_busy(void)
{
    if(motor1.state == MOTOR_RUNNING) return 1;
    if(motor2.state == MOTOR_RUNNING) return 1;

    return 0;
}

/* =========================
   ��ʱ���
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

