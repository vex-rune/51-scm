/*
 * File:    uart.h
 * Project: 06-uart-pc
 * MCU:     STC89C52RC (11.0592 MHz)
 *
 * 本模块提供 6 个功能:
 *   - Uart_Init()      : 初始化 (含定时器0 用作超时计时)
 *   - Uart_SendByte(c) : 发 1 个字节
 *   - Uart_SendString("hello") : 发字符串
 *   - Uart_Available() : 是否收到数据 (1=有 0=没有)
 *   - Uart_ReadByte()  : 读 1 个字节 (返回错误码时为 0xFF)
 *   - Uart_ReadLine(buf, size) : 读一行 (直到 \n 或超时)
 *
 * 硬件: USB 转 TTL 接 P3.0(RXD)/P3.1(TXD),GND 共接
 * 串口助手: 9600 / 8N1
 */

#ifndef __UART_H__
#define __UART_H__

#include <stc89c52rc.h>

/* 波特率: 9600 / 19200 / 115200 */
#define UART_BAUD  9600

/* 接收缓冲区大小 (字节) */
#define UART_RX_BUF_SIZE  32

/* 读取超时: 1 字节约 1ms, 3 字节约 3ms */
#define UART_TIMEOUT_MS  3

/* 错误码: Uart_ReadByte 超时时返回 0xFF */
#define UART_READ_ERR  0xFF

/* 函数声明 */
void Uart_Init(void);
void Uart_SendByte(unsigned char dat);
void Uart_SendString(const char *s);
bit Uart_Available(void);
unsigned char Uart_ReadByte(void);

#endif /* __UART_H__ */