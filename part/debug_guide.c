/**
 * ============================================================
 *  调试步骤 — 按顺序排查
 * ============================================================
 *
 * 第一步：全量重编译
 *   删除 build/F407/ 下所有 .o 文件，MDK 里点 Rebuild All
 *
 * 第二步：最简测试
 *   PC 只发 3 行：
 *     G21
 *     G90
 *     G00 X10 Y10
 *   等电机走完，停下看调试器
 *
 * 第三步：按这个顺序读计数器
 *
 *   [PC→STM32 收数据]
 *   dbg_serial_rx   —— 应该 == 3
 *   dbg_parse_ok    —— 应该 == 3 (G21,G90 OK + G0入队OK)
 *   dbg_parse_fail  —— 应该 == 0
 *
 *   [队列]
 *   dbg_queue_push  —— 应该 == 1 (只有G0入队)
 *   dbg_queue_full  —— 应该 == 0
 *
 *   [规划器]
 *   dbg_planner_pop  —— 应该 == 1
 *   dbg_planner_busy —— 运动过程中应该有，运动结束后不再增长
 *   dbg_go_called    —— 应该 == 1 ← 关键！
 *
 *   [电机通信]
 *   dbg_motor_poll   —— > 0 (一直在查)
 *   dbg_usart1_idle  —— > 0 (电机有回复)
 *   dbg_usart1_len   —— 应该有值 (上次显示0很可疑)
 *   dbg_frame_parse  —— > 0 (解析到帧)
 *   dbg_run_found    —— > 0 (电机报告RUNNING)
 *   dbg_idle_found   —— >= 1 (电机最终报告IDLE)
 *   dbg_motor1_state —— 最终应该是 0 (IDLE)
 *   dbg_motor2_state —— 最终应该是 0 (IDLE)
 *
 * 第四步：看电机是否真的动了
 *   如果 dbg_go_called == 0 但电机动了 → 编译链接有问题，Rebuild All
 *   如果 dbg_go_called == 0 且电机没动 → go() 没被调用，往上查 planner
 *   如果 dbg_go_called == 1 且电机动了但没有 IDLE → 电机回复/解析问题
 *
 * 第五步：检查 USART1 接收
 *   如果 dbg_usart1_len 一直是 0：
 *     → USART1 DMA 没收到数据
 *     → 检查 RS-485 方向控制引脚
 *     → 检查电机 TX 是否连到 STM32 USART1 RX (PA10)
 *     → 用示波器看 PA10 有没有波形
 *
 * ============================================================
 *  最可能的问题排序
 * ============================================================
 *
 *  1. 编译不全量 → 旧的 .o 导致调试计数器不准确
 *  2. USART1 RX 没收到数据 → 电机命令可能发出去了但回复没回来
 *  3. motor_poll 用了 Emm_V5_Read_Sys_Params → 和 go() 的 TX HAL状态冲突
 *     如果 HAL TX 锁未释放，query 发不出去，电机不回
 */
