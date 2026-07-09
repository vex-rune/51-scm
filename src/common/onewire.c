/*
 * File:    onewire.c
 * Project: 公共模块 - 1-Wire 软件模拟驱动实现
 * MCU:     STC89C52RC (11.0592 MHz, 1 机器周期 ≈ 1.085us)
 * Brief:   基于 GPIO 位操作实现 1-Wire 时序
 *
 * 参考: DS18B20 Datasheet + 实测验证的 8051 时序参数
 * DQ: P44 (STC89C52RC 扩展端口)
 */

#define ONEWIRE_C
#include <stc89c52rc.h>
#include "onewire.h"

/* P44 位地址 = 0xE8 (P4) + 4 = 0xEC */
#define ONEWIRE_DQ_BIT  0xEC
#define ONEWIRE_DQ      P44

/* 延时宏: 11.0592MHz 下 1 机器周期 = 1.085us */
#define DELAY_1US()  { __asm nop __endasm; }
#define DELAY_5US()  { DELAY_1US(); DELAY_1US(); DELAY_1US(); DELAY_1US(); DELAY_1US(); }
#define DELAY_10US() { DELAY_5US(); DELAY_5US(); }

/**
 * @brief  粗略微秒延时 (用于长延时)
 */
static void delay_us(unsigned char us)
{
    while (us--) {
        DELAY_1US();
    }
}

/**
 * @brief  毫秒延时
 */
static void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);  /* 约 1ms @ 11.0592MHz */
}

/* ============================================================
 * API 实现
 * ============================================================ */

/**
 * @brief  初始化: 释放 DQ
 */
void OneWire_Init(void)
{
    ONEWIRE_DQ = 1;
}

/**
 * @brief  复位脉冲 + 检测应答
 * @return 1 检测到从机, 0 无应答
 *
 * 时序: 主机拉低 480-960us -> 释放 -> 等 15-60us 采样应答
 */
unsigned char OneWire_Reset(void)
{
    unsigned char ack;

    /* 主机拉低 500us */
    ONEWIRE_DQ = 0;
    delay_us(250);  /* ~270us */
    delay_us(250);  /* ~540us 总计 */

    /* 释放, 等待从机应答 */
    ONEWIRE_DQ = 1;
    delay_us(30);   /* ~32us, 落在 15-60us 应答窗口 */

    ack = ONEWIRE_DQ;  /* 0 = 从机拉低应答 */

    /* 等待从机释放 (总恢复时间 480us) */
    delay_us(200);
    delay_us(200);

    return (ack == 0) ? 1 : 0;
}

/**
 * @brief  写 1 字节 (低位先发)
 * @param  dat  要写的字节
 *
 * 写 1: 拉低 <15us -> 释放 -> 等 60us
 * 写 0: 拉低 60us -> 释放
 */
void OneWire_WriteByte(unsigned char dat)
{
    unsigned char i;

    for (i = 0; i < 8; i++) {
        if (dat & 0x01) {
            /* 写 1 */
            ONEWIRE_DQ = 0;
            DELAY_5US();     /* ~5us */
            ONEWIRE_DQ = 1;
            delay_us(55);    /* ~60us 恢复 */
        } else {
            /* 写 0 */
            ONEWIRE_DQ = 0;
            delay_us(55);    /* ~60us 拉低 */
            ONEWIRE_DQ = 1;
            DELAY_5US();     /* ~5us 恢复 */
        }
        dat >>= 1;
    }
}

/**
 * @brief  读 1 字节 (低位先收)
 * @return 读到的字节
 *
 * 时序: 拉低 >1us -> 释放 -> 等 10us 采样 -> 等 50us
 */
unsigned char OneWire_ReadByte(void)
{
    unsigned char i, dat = 0;

    for (i = 0; i < 8; i++) {
        dat >>= 1;

        /* 启动读时隙 */
        ONEWIRE_DQ = 0;
        DELAY_1US();     /* ~1us */
        DELAY_1US();     /* ~2us */
        ONEWIRE_DQ = 1;  /* 释放 */

        /* 延时到采样点 (~10us 从下降沿) */
        DELAY_5US();     /* ~7us */
        DELAY_1US();     /* ~8us */
        DELAY_1US();     /* ~9us */

        /* 采样 */
        if (ONEWIRE_DQ) {
            dat |= 0x80;
        }

        /* 等待时隙结束 (总 60us) */
        delay_us(50);
    }

    return dat;
}
