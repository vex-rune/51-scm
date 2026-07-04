/*
 * File:    main.c
 * Project: 06-uart-pc
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 *
 * 行为:
 *   1. 上电发欢迎信息
 *   2. 收到 PC 发的字节, 原样回发 (回显)
 *   3. 如果收到 '1' -> 点亮 P1.0 LED
 *   4. 如果收到 '0' -> 熄灭 P1.0 LED
 *
 * 接收规则 (由 uart.c 实现):
 *   - 以 \n 为结束符 (一行结束)
 *   - 超过 3ms 没收到字节算超时
 *   - 超时或读到 \n 后清空缓冲
 *
 * 硬件: USB 转 TTL 接 P3.0(RXD)/P3.1(TXD),GND 共接
 * 串口助手: 9600 / 8N1, 发送时勾选"加回车换行"
 */

#include <stc89c52rc.h>
#include "uart.h"

#define LED_PIN  P1_0

void main(void)
{
    unsigned char c;

    LED_PIN = 1;                       /* LED 初始熄灭 */
    Uart_Init();                       /* 初始化串口 + 定时器 0 (超时) */

    Uart_SendString("=== 06-uart-pc ready ===\r\n");
    Uart_SendString("Send '1' or '0' to control LED.\r\n");

    while (1) {
        if (Uart_Available()) {
            c = Uart_ReadByte();

            if (c == UART_READ_ERR) {
                /* 超时或读取异常 */
                Uart_SendString("[timeout]\r\n");
                continue;
            }

            if (c == '1') {
                LED_PIN = 0;
                Uart_SendString("LED ON\r\n");
            }
            else if (c == '0') {
                LED_PIN = 1;
                Uart_SendString("LED OFF\r\n");
            }
            else {
                Uart_SendByte(c);      /* 回显 */
            }
        }
    }
}