#include "Emm_V5.h"

__IO uint16_t MMCL_count = 0, MMCL_cmd[MMCL_LEN] = {0};

/**********************************************************
*** 触发动作命令
**********************************************************/
/**
  * @brief    触发编码器校准
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Trig_Encoder_Cal(uint8_t addr)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x06;                       // 功能码
  cmd[2] =  0x45;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
	HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, 4);
}



/**********************************************************
*** 运动控制命令
**********************************************************/
/**
  * @brief    多电机命令（Y42）
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Multi_Motor_Cmd(uint8_t addr)
{
  uint16_t i = 0, j = 0, len = 0; __IO static uint8_t cmd[MMCL_LEN] = {0};
  
	// 多电机命令长度大于0
	if(MMCL_count > 0)
	{
		// 多电机命令的总字节数
		len = MMCL_count + 5;
		
		// 装载命令
		cmd[0] = addr;                       // 地址
		cmd[1] = 0xAA;                       // 功能码
		cmd[2] = (uint8_t)(len >> 8);				 // 总字节数高8位
		cmd[3] = (uint8_t)(len); 		 				 // 总字节数低8位
		for(i=0,j=4; i < MMCL_count; i++,j++) { cmd[j] = MMCL_cmd[i]; }
		cmd[j] = 0x6B; ++j;                  // 校验字节
		
		// 发送命令
		HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, j); MMCL_count = 0;
	}
	else
	{
		MMCL_count = 0;
	}
}

/**
  * @brief    使能信号控制
  * @param    addr  ：电机地址
  * @param    state ：使能状态     ，true为使能电机，false为关闭电机
  * @param    snF   ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_En_Control(uint8_t addr, bool state, bool snF)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF3;                       // 功能码
  cmd[2] =  0xAB;                       // 辅助码
  cmd[3] =  (uint8_t)state;             // 使能状态
  cmd[4] =  snF;                        // 多机同步运动标志
  cmd[5] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, 6);
}

/**
  * @brief    位置模式
  * @param    addr：电机地址
  * @param    dir ：方向        ，0为CW，其余值为CCW
  * @param    vel ：速度(RPM)   ，范围0 - 5000RPM
  * @param    acc ：加速度      ，范围0 - 255，注意：0是直接启动
  * @param    clk ：脉冲数      ，范围0- (2^32 - 1)个
  * @param    raF ：相位/绝对标志，false为相对运动，true为绝对值运动
  * @param    snF ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF)
{
  __IO static uint8_t cmd[16] = {0};

  // 装载命令
  cmd[0]  =  addr;                      // 地址
  cmd[1]  =  0xFD;                      // 功能码
  cmd[2]  =  dir;                       // 方向
  cmd[3]  =  (uint8_t)(vel >> 8);       // 速度(RPM)高8位字节
  cmd[4]  =  (uint8_t)(vel >> 0);       // 速度(RPM)低8位字节 
  cmd[5]  =  acc;                       // 加速度，注意：0是直接启动
  cmd[6]  =  (uint8_t)(clk >> 24);      // 脉冲数(bit24 - bit31)
  cmd[7]  =  (uint8_t)(clk >> 16);      // 脉冲数(bit16 - bit23)
  cmd[8]  =  (uint8_t)(clk >> 8);       // 脉冲数(bit8  - bit15)
  cmd[9]  =  (uint8_t)(clk >> 0);       // 脉冲数(bit0  - bit7 )
  cmd[10] =  raF;                       // 相位/绝对标志，false为相对运动，true为绝对值运动
  cmd[11] =  snF;                       // 多机同步运动标志，false为不启用，true为启用
  cmd[12] =  0x6B;                      // 校验字节
  
  // 发送命令
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, 13);
}

/**
  * @brief    立即停止
  * @param    addr  ：电机地址
  * @param    snF   ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Stop_Now(uint8_t addr, bool snF)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFE;                       // 功能码
  cmd[2] =  0x98;                       // 辅助码
  cmd[3] =  snF;                        // 多机同步运动标志
  cmd[4] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, 5);
}

/**
  * @brief    多机同步运动
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Synchronous_motion(uint8_t addr)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFF;                       // 功能码
  cmd[2] =  0x66;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit_DMA(&huart1, (uint8_t *)cmd, 4);
}

/**********************************************************
*** 原点回零命令
**********************************************************/


/**
***********************************************************
***********************************************************
*** 
***
*** @brief	以下是把相应命令加载到Y42多电机命令上的函数（Y42）
***
*** 
***********************************************************
***********************************************************
***/
/**********************************************************
*** 触发动作命令
**********************************************************/
/**
  * @brief    触发编码器校准 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Trig_Encoder_Cal(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x06;                       // 功能码
  cmd[2] =  0x45;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    重启电机（Y42） - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Reset_Motor(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x08;                       // 功能码
  cmd[2] =  0x97;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    将当前位置清零 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Reset_CurPos_To_Zero(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0A;                       // 功能码
  cmd[2] =  0x6D;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    解除堵转保护 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Reset_Clog_Pro(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0E;                       // 功能码
  cmd[2] =  0x52;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    恢复出厂设置 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Restore_Motor(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0F;                       // 功能码
  cmd[2] =  0x5F;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**********************************************************
*** 运动控制命令
**********************************************************/
/**
  * @brief    使能信号控制 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @param    state ：使能状态     ，true为使能电机，false为关闭电机
  * @param    snF   ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_En_Control(uint8_t addr, bool state, bool snF)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF3;                       // 功能码
  cmd[2] =  0xAB;                       // 辅助码
  cmd[3] =  (uint8_t)state;             // 使能状态
  cmd[4] =  snF;                        // 多机同步运动标志
  cmd[5] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 6; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    速度模式 - 加载到多电机指令上
  * @param    addr：电机地址
  * @param    dir ：方向       ，0为CW，其余值为CCW
  * @param    vel ：速度       ，范围0 - 5000RPM
  * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
  * @param    snF ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Vel_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
  uint8_t j = 0, cmd[16] = {0};

  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF6;                       // 功能码
  cmd[2] =  dir;                        // 方向
  cmd[3] =  (uint8_t)(vel >> 8);        // 速度(RPM)高8位字节
  cmd[4] =  (uint8_t)(vel >> 0);        // 速度(RPM)低8位字节
  cmd[5] =  acc;                        // 加速度，注意：0是直接启动
  cmd[6] =  snF;                        // 多机同步运动标志
  cmd[7] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 8; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    位置模式 - 加载到多电机指令上
  * @param    addr：电机地址
  * @param    dir ：方向        ，0为CW，其余值为CCW
  * @param    vel ：速度(RPM)   ，范围0 - 5000RPM
  * @param    acc ：加速度      ，范围0 - 255，注意：0是直接启动
  * @param    clk ：脉冲数      ，范围0- (2^32 - 1)个
  * @param    raF ：相位/绝对标志，false为相对运动，true为绝对值运动
  * @param    snF ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF)
{
  uint8_t j = 0, cmd[16] = {0};

  // 装载命令
  cmd[0]  =  addr;                      // 地址
  cmd[1]  =  0xFD;                      // 功能码
  cmd[2]  =  dir;                       // 方向
  cmd[3]  =  (uint8_t)(vel >> 8);       // 速度(RPM)高8位字节
  cmd[4]  =  (uint8_t)(vel >> 0);       // 速度(RPM)低8位字节 
  cmd[5]  =  acc;                       // 加速度，注意：0是直接启动
  cmd[6]  =  (uint8_t)(clk >> 24);      // 脉冲数(bit24 - bit31)
  cmd[7]  =  (uint8_t)(clk >> 16);      // 脉冲数(bit16 - bit23)
  cmd[8]  =  (uint8_t)(clk >> 8);       // 脉冲数(bit8  - bit15)
  cmd[9]  =  (uint8_t)(clk >> 0);       // 脉冲数(bit0  - bit7 )
  cmd[10] =  raF;                       // 相位/绝对标志，false为相对运动，true为绝对值运动
  cmd[11] =  snF;                       // 多机同步运动标志，false为不启用，true为启用
  cmd[12] =  0x6B;                      // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 13; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    立即停止 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @param    snF   ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Stop_Now(uint8_t addr, bool snF)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFE;                       // 功能码
  cmd[2] =  0x98;                       // 辅助码
  cmd[3] =  snF;                        // 多机同步运动标志
  cmd[4] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 5; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    多机同步运动 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Synchronous_motion(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFF;                       // 功能码
  cmd[2] =  0x66;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**********************************************************
*** 原点回零命令
**********************************************************/
/**
  * @brief    设置单圈回零的零点位置 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @param    svF   ：是否存储标志，false为不存储，true为存储
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Origin_Set_O(uint8_t addr, bool svF)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x93;                       // 功能码
  cmd[2] =  0x88;                       // 辅助码
  cmd[3] =  svF;                        // 是否存储标志，false为不存储，true为存储
  cmd[4] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 5; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    触发回零 - 加载到多电机指令上
  * @param    addr   ：电机地址
  * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  * @param    snF   ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Origin_Trigger_Return(uint8_t addr, uint8_t o_mode, bool snF)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x9A;                       // 功能码
  cmd[2] =  o_mode;                     // 回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  cmd[3] =  snF;                        // 多机同步运动标志，false为不启用，true为启用
  cmd[4] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 5; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    强制中断并退出回零 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Origin_Interrupt(uint8_t addr)
{
  uint8_t j = 0, cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x9C;                       // 功能码
  cmd[2] =  0x48;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 4; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    修改回零参数 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @param    svF   ：是否存储标志，false为不存储，true为存储
  * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  * @param    o_dir  ：回零方向，0为CW，其余值为CCW
  * @param    o_vel  ：回零速度，单位：RPM（转/分钟）
  * @param    o_tm   ：回零超时时间，单位：毫秒
  * @param    sl_vel ：无限位碰撞回零检测转速，单位：RPM（转/分钟）
  * @param    sl_ma  ：无限位碰撞回零检测电流，单位：Ma（毫安）
  * @param    sl_ms  ：无限位碰撞回零检测时间，单位：Ms（毫秒）
  * @param    potF   ：上电自动触发回零，false为不使能，true为使能
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Origin_Modify_Params(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF)
{
  uint8_t j = 0, cmd[32] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x4C;                       // 功能码
  cmd[2] =  0xAE;                       // 辅助码
  cmd[3] =  svF;                        // 是否存储标志，false为不存储，true为存储
  cmd[4] =  o_mode;                     // 回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  cmd[5] =  o_dir;                      // 回零方向
  cmd[6]  =  (uint8_t)(o_vel >> 8);     // 回零速度(RPM)高8位字节
  cmd[7]  =  (uint8_t)(o_vel >> 0);     // 回零速度(RPM)低8位字节 
  cmd[8]  =  (uint8_t)(o_tm >> 24);     // 回零超时时间(bit24 - bit31)
  cmd[9]  =  (uint8_t)(o_tm >> 16);     // 回零超时时间(bit16 - bit23)
  cmd[10] =  (uint8_t)(o_tm >> 8);      // 回零超时时间(bit8  - bit15)
  cmd[11] =  (uint8_t)(o_tm >> 0);      // 回零超时时间(bit0  - bit7 )
  cmd[12] =  (uint8_t)(sl_vel >> 8);    // 无限位碰撞回零检测转速(RPM)高8位字节
  cmd[13] =  (uint8_t)(sl_vel >> 0);    // 无限位碰撞回零检测转速(RPM)低8位字节 
  cmd[14] =  (uint8_t)(sl_ma >> 8);     // 无限位碰撞回零检测电流(Ma)高8位字节
  cmd[15] =  (uint8_t)(sl_ma >> 0);     // 无限位碰撞回零检测电流(Ma)低8位字节 
  cmd[16] =  (uint8_t)(sl_ms >> 8);     // 无限位碰撞回零检测时间(Ms)高8位字节
  cmd[17] =  (uint8_t)(sl_ms >> 0);     // 无限位碰撞回零检测时间(Ms)低8位字节
  cmd[18] =  potF;                      // 上电自动触发回零，false为不使能，true为使能
  cmd[19] =  0x6B;                      // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < 20; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**********************************************************
*** 读取系统参数命令
**********************************************************/
/**********************************************************
*** 读取系统参数命令
**********************************************************/
/**
  * @brief    定时返回信息命令（Y42）
  * @param    addr  	：电机地址
  * @param    s     	：系统参数类型
	* @param    time_ms ：定时时间
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Auto_Return_Sys_Params_Timed(uint8_t addr, SysParams_t s, uint16_t time_ms)
{
  uint8_t i = 0, j = 0; uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[i] = addr; ++i;                   // 地址

  cmd[i] = 0x11; ++i;                   // 功能码

  cmd[i] = 0x18; ++i;                   // 辅助码

  switch(s)                             // 信息功能码
  {
    case S_VBUS : cmd[i] = 0x24; ++i; break;	// 读取总线电压
		case S_CBUS : cmd[i] = 0x26; ++i; break;	// 读取总线电流
    case S_CPHA : cmd[i] = 0x27; ++i; break;	// 读取相电流
		case S_ENCO : cmd[i] = 0x29; ++i; break;	// 读取编码器原始值
		case S_CLKC : cmd[i] = 0x30; ++i; break;	// 读取实时脉冲数
    case S_ENCL : cmd[i] = 0x31; ++i; break;	// 读取经过线性化校准后的编码器值
		case S_CLKI : cmd[i] = 0x32; ++i; break;	// 读取输入脉冲数
    case S_TPOS : cmd[i] = 0x33; ++i; break;	// 读取电机目标位置
    case S_SPOS : cmd[i] = 0x34; ++i; break;	// 读取电机实时设定的目标位置
		case S_VEL  : cmd[i] = 0x35; ++i; break;	// 读取电机实时转速
    case S_CPOS : cmd[i] = 0x36; ++i; break;	// 读取电机实时位置
    case S_PERR : cmd[i] = 0x37; ++i; break;	// 读取电机位置误差
		case S_VBAT : cmd[i] = 0x38; ++i; break;	// 读取多圈编码器电池电压（Y42）
		case S_TEMP : cmd[i] = 0x39; ++i; break;	// 读取电机实时温度（Y42）
    case S_FLAG : cmd[i] = 0x3A; ++i; break;	// 读取电机状态标志位
    case S_OFLAG: cmd[i] = 0x3B; ++i; break;	// 读取回零状态标志位
		case S_OAF  : cmd[i] = 0x3C; ++i; break;	// 读取电机状态标志位 + 回零状态标志位（Y42）
		case S_PIN  : cmd[i] = 0x3D; ++i; break;	// 读取引脚状态（Y42）
    default: break;
  }
	
	cmd[i] = (uint8_t)(time_ms >> 8);  ++i;	// 定时时间
	cmd[i] = (uint8_t)(time_ms >> 0);  ++i;

  cmd[i] = 0x6B; ++i;                   	// 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < i; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**
  * @brief    读取系统参数 - 加载到多电机指令上
  * @param    addr  ：电机地址
  * @param    s     ：系统参数类型
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_MMCL_Read_Sys_Params(uint8_t addr, SysParams_t s)
{
  uint8_t i = 0, j = 0; uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[i] = addr; ++i;                   // 地址

  switch(s)                             // 功能码
  {
    case S_VBUS : cmd[i] = 0x24; ++i; break;	// 读取总线电压
		case S_CBUS : cmd[i] = 0x26; ++i; break;	// 读取总线电流
    case S_CPHA : cmd[i] = 0x27; ++i; break;	// 读取相电流
		case S_ENCO : cmd[i] = 0x29; ++i; break;	// 读取编码器原始值
		case S_CLKC : cmd[i] = 0x30; ++i; break;	// 读取实时脉冲数
    case S_ENCL : cmd[i] = 0x31; ++i; break;	// 读取经过线性化校准后的编码器值
		case S_CLKI : cmd[i] = 0x32; ++i; break;	// 读取输入脉冲数
    case S_TPOS : cmd[i] = 0x33; ++i; break;	// 读取电机目标位置
    case S_SPOS : cmd[i] = 0x34; ++i; break;	// 读取电机实时设定的目标位置
		case S_VEL  : cmd[i] = 0x35; ++i; break;	// 读取电机实时转速
    case S_CPOS : cmd[i] = 0x36; ++i; break;	// 读取电机实时位置
    case S_PERR : cmd[i] = 0x37; ++i; break;	// 读取电机位置误差
		case S_VBAT : cmd[i] = 0x38; ++i; break;	// 读取多圈编码器电池电压（Y42）
		case S_TEMP : cmd[i] = 0x39; ++i; break;	// 读取电机实时温度（Y42）
    case S_FLAG : cmd[i] = 0x3A; ++i; break;	// 读取电机状态标志位
    case S_OFLAG: cmd[i] = 0x3B; ++i; break;	// 读取回零状态标志位
		case S_OAF  : cmd[i] = 0x3C; ++i; break;	// 读取电机状态标志位 + 回零状态标志位（Y42）
		case S_PIN  : cmd[i] = 0x3D; ++i; break;	// 读取引脚状态（Y42）
    default: break;
  }

  cmd[i] = 0x6B; ++i;                   // 校验字节
  
  // 加载当前命令到多电机命令中
  for(j=0; j < i; j++) { MMCL_cmd[MMCL_count] = cmd[j]; ++MMCL_count; }
}

/**********************************************************
*** 读写驱动参数命令
**********************************************************/
