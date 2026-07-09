/*
 * File:    uart.h
 * Project: 公共模块 - 串口通信
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   UART1 字节流收发模块 + 软件 tick 节拍
 *
 * 特性：
 *   - 模式1, 8N1, 默认 9600 bps（可在 uart.c 修改重载值）
 *   - 发送：阻塞式，调用 Uart_SendByte/Str 会自旋等待上次发送完成
 *   - 接收：环形缓冲（默认 32 字节），中断写入、主循环读取
 *   - 接收溢出时丢弃新数据，不覆盖已有数据
 *   - 内置 10ms 软件 tick（基于 T2）用于超时判定
 *
 * 资源占用：
 *   - T1（波特率发生器）
 *   - T2（10ms tick, 65536 - 9216, 自动重装）
 *   - 中断向量 4（UART）
 *   - RAM: 接收缓冲 + tick 计数(约 40 字节)
 *
 * 与其他模块的兼容性：
 *   - 不占用 T0，可与 timer / smg 模块共存
 *   - 占用 T2，不能与其它使用 T2 的模块同时使用
 */

#ifndef __UART_H__
#define __UART_H__

/* ============================================================
 * 可调参数
 * ============================================================ */
#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE  8         /* 接收环形缓冲大小（字节） - 减小以节省RAM */
#endif

/* tick 周期: 10ms (T2 中断固定周期, 由 11.0592MHz / 12 / 9216 决定) */
#define UART_TICK_MS      10

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化 UART（占用 T1 波特率 + T2 tick + 中断）
 * @note   调用后开启全局中断 EA=1
 */
void Uart_Init(void);

/**
 * @brief  发送一个字节（阻塞：等待 TI 中断清忙标志）
 * @param  ch  要发送的字节
 */
void Uart_SendByte(unsigned char ch);

/**
 * @brief  发送以 '\0' 结尾的字符串
 * @param  s   C 字符串指针
 */
void Uart_SendStr(const char *s);

/**
 * @brief  发送指定长度的二进制数据
 * @param  data  数据指针
 * @param  len   长度(字节)
 */
void Uart_SendBytes(const unsigned char *data, unsigned char len);

/**
 * @brief  查询接收缓冲中是否有未读数据
 * @return 1=有数据 0=空
 */
unsigned char Uart_Available(void);

/**
 * @brief  从接收缓冲读取一个字节
 * @param  ch  输出参数，返回读到的字节
 * @return 1=成功读到 0=缓冲空
 */
unsigned char Uart_ReadByte(unsigned char *ch);

/**
 * @brief  查看接收缓冲中最早的一个字节（不移除）
 * @param  ch  输出参数
 * @return 1=成功 0=缓冲空
 */
unsigned char Uart_Peek(unsigned char *ch);

/**
 * @brief  清空接收缓冲
 */
void Uart_FlushRx(void);

/**
 * @brief  返回接收缓冲中当前未读字节数
 */
unsigned char Uart_RxCount(void);

/**
 * @brief  返回自 Uart_Init 以来的 tick 数（每 UART_TICK_MS 增 1）
 * @note   用于应用层实现超时/防抖等时序判定
 */
unsigned int Uart_GetTick(void);

/**
 * @brief  返回自上次成功接收以来空闲的 tick 数
 * @note   主循环/超时判定使用此 API 比 Uart_GetTick 更便利
 */
unsigned int Uart_GetIdleTicks(void);

/**
 * @brief  UART 中断服务程序（向量 4）
 * @note   不应被用户直接调用
 */
void Uart_ISR(void) __interrupt(4);

/**
 * @brief  T2 中断服务程序（向量 5）
 * @note   不应被用户直接调用
 */
void Uart_T2_ISR(void) __interrupt(5);

#endif /* __UART_H__ */