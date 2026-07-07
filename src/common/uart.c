/*
 * File:    uart.c
 * Project: 公共模块 - 串口通信
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   UART1 字节流收发实现 + T2 软件 tick
 *
 * 内部机制：
 *   - T1 模式2(8 位自动重装) 作为波特率发生器
 *     TH1=0xFD 在 11.0592MHz 下产生 9600bps (SMOD=0)
 *   - T2 模式1(16 位自动重装) 作为 10ms 节拍定时器
 *     RCAP2 = 65536 - 9216, 11.0592MHz / 12 / 9216 = 100Hz
 *     每 10 次中断 = 1 tick, 主循环 / 应用层用 Uart_GetTick() 读
 *   - 接收: 中断将字节写入环形缓冲 rx_buf
 *           主循环通过 Uart_ReadByte 取字节
 *   - 发送: 写 SBUF 后硬件启动移位输出
 *           TI 中断清 uart_tx_busy, 唤醒等待者
 *
 * 线程安全：
 *   - 接收缓冲由中断写入, 主循环读取, 二者通过 rx_head/rx_tail 同步
 *   - rx_head 仅在 ISR 中写, 主循环只读
 *   - rx_tail 仅在主循环中写
 *   - uart_tx_busy 由 ISR 写 0, 主循环写 1, volatile __bit 保证可见性
 *   - tick 计数 / last_rx_tick 由 T2 ISR 写, 主循环读
 *     用 EA=0/1 临界区保护或读两次取一致值
 */

#include "uart.h"
#include <stc89c52rc.h>

/* ============================================================
 * 内部常量与计算
 * ============================================================ */

/* T2 重载值: 11.0592MHz / 12 = 921600 机器周期/秒
 * 10ms 需 9216 个机器周期, 16 位重载 = 65536 - 9216 = 56320 */
#define T2_RELOAD       (65536UL - 9216UL)

/* ============================================================
 * 内部存储
 * ============================================================ */

/* 发送忙标志: 1=正在移位输出, 0=发送完成 */
static volatile __bit uart_tx_busy = 0;

/* 接收环形缓冲 */
static volatile unsigned char rx_buf[UART_RX_BUF_SIZE];
static volatile unsigned char rx_head = 0;   /* 写索引, 仅 UART ISR 写 */
static volatile unsigned char rx_tail = 0;   /* 读索引, 仅主循环写 */

/* tick 节拍(T2 ISR 每 UART_TICK_MS 毫秒增 1) */
static volatile unsigned int  g_tick = 0;            /* 自 Uart_Init 起的总 tick 数 */
static volatile unsigned int  last_rx_tick = 0;      /* 上次接收到字节的 tick 时间戳 */

/* ============================================================
 * 初始化
 * ============================================================ */
void Uart_Init(void)
{
    /* —— 串口模式 1 (8N1) —— */
    SM0 = 0;
    SM1 = 1;

    /* SMOD = 0, 不倍速 */
    PCON &= 0x7F;

    /* —— T1: 模式2, 8 位自动重装, 波特率发生器 —— */
    TMOD &= 0x0F;
    TMOD |= 0x20;
    TL1 = 0xFD;     /* 11.0592MHz / 12 / 32 / (256-0xFD) ≈ 9600 bps */
    TH1 = 0xFD;     /* 重装值 */
    TR1 = 1;        /* 启动 T1 */

    /* —— T2: 模式1, 16 位自动重装, UART_TICK_MS ms 节拍 —— */
    T2CON = 0;      /* 清控制位, 默认工作于定时模式 */
    T2MOD = 0;      /* 默认 */
    RCAP2L = (unsigned char)(T2_RELOAD & 0xFF);
    RCAP2H = (unsigned char)(T2_RELOAD >> 8);
    TL2 = (unsigned char)(T2_RELOAD & 0xFF);
    TH2 = (unsigned char)(T2_RELOAD >> 8);
    TR2 = 1;        /* 启动 T2 */
    ET2 = 1;        /* 允许 T2 中断 */

    /* —— 串口接收 —— */
    REN = 1;
    SM2 = 0;
    RI = 0;
    TI = 0;

    /* —— 清 tick —— */
    g_tick = 0;
    last_rx_tick = 0;

    /* —— 开中断 —— */
    EA = 1;
    ES = 1;
}

/* ============================================================
 * 发送
 * ============================================================ */
void Uart_SendByte(unsigned char ch)
{
    while (uart_tx_busy) {
        /* 自旋等待 TI 中断清忙标志 */
    }
    uart_tx_busy = 1;
    SBUF = ch;
}

void Uart_SendStr(const char *s)
{
    while (*s) {
        Uart_SendByte((unsigned char)*s++);
    }
}

void Uart_SendBytes(const unsigned char *data, unsigned char len)
{
    while (len--) {
        Uart_SendByte(*data++);
    }
}

/* ============================================================
 * 接收
 * ============================================================ */
unsigned char Uart_Available(void)
{
    return (rx_head != rx_tail) ? 1 : 0;
}

unsigned char Uart_ReadByte(unsigned char *ch)
{
    if (rx_head == rx_tail) {
        return 0;
    }
    *ch = rx_buf[rx_tail];
    rx_tail = (unsigned char)((rx_tail + 1) % UART_RX_BUF_SIZE);
    return 1;
}

unsigned char Uart_Peek(unsigned char *ch)
{
    if (rx_head == rx_tail) {
        return 0;
    }
    *ch = rx_buf[rx_tail];
    return 1;
}

void Uart_FlushRx(void)
{
    /* 关中断避免与 ISR 冲突 */
    EA = 0;
    rx_head = 0;
    rx_tail = 0;
    EA = 1;
}

unsigned char Uart_RxCount(void)
{
    return (unsigned char)((rx_head - rx_tail + UART_RX_BUF_SIZE) % UART_RX_BUF_SIZE);
}

/* ============================================================
 * Tick / 超时辅助
 * ============================================================ */
unsigned int Uart_GetTick(void)
{
    /* 读 16 位 tick 在 8 位 MCU 上非原子, 此处读两次取一致值 */
    unsigned int a, b;
    do {
        a = g_tick;
        b = g_tick;
    } while (a != b);
    return a;
}

unsigned int Uart_GetIdleTicks(void)
{
    return (unsigned int)(Uart_GetTick() - last_rx_tick);
}

/* ============================================================
 * 中断服务程序
 * ============================================================ */
void Uart_ISR(void) __interrupt(4)
{
    unsigned char next;

    if (RI) {
        RI = 0;

        /* 留一格区分满/空 */
        next = (unsigned char)((rx_head + 1) % UART_RX_BUF_SIZE);
        if (next != rx_tail) {
            rx_buf[rx_head] = SBUF;
            rx_head = next;
        }

        /* 记录接收时间戳(用于空闲 tick 统计) */
        last_rx_tick = g_tick;
    }

    if (TI) {
        TI = 0;
        uart_tx_busy = 0;
    }
}

void Uart_T2_ISR(void) __interrupt(5)
{
    /* T2 模式1 中断需软件清 TF2 */
    TF2 = 0;
    g_tick++;
}