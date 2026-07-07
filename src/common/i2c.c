/*
 * File:    i2c.c
 * Project: 公共模块 - I2C 软件模拟驱动实现
 * MCU:     STC89C52RC
 * Brief:   提供 I2C 基础读写原语
 */

#include <stc89c52rc.h>
#include <delay.h>
#include "i2c.h"

/**
 * @brief  初始化 I2C GPIO
 */
void I2C_Init(void)
{
    I2C_SCL = 1;
    I2C_SDA = 1;
}

/**
 * @brief  发送起始信号
 */
void I2C_Start(void)
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    DelayUs(I2C_DELAY_US);
    I2C_SDA = 0;
    DelayUs(I2C_DELAY_US);
    I2C_SCL = 0;
}

/**
 * @brief  发送停止信号
 */
void I2C_Stop(void)
{
    I2C_SDA = 0;
    I2C_SCL = 1;
    DelayUs(I2C_DELAY_US);
    I2C_SDA = 1;
    DelayUs(I2C_DELAY_US);
}

/**
 * @brief  发送一个字节
 */
void I2C_SendByte(unsigned char dat)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        I2C_SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        DelayUs(I2C_DELAY_US);
        I2C_SCL = 1;
        DelayUs(I2C_DELAY_US);
        I2C_SCL = 0;
    }
    I2C_SDA = 1;  /* 释放 SDA */
}

/**
 * @brief  接收一个字节
 */
unsigned char I2C_RecvByte(void)
{
    unsigned char i, dat = 0;
    I2C_SDA = 1;
    for (i = 0; i < 8; i++) {
        I2C_SCL = 0;
        DelayUs(I2C_DELAY_US);
        I2C_SCL = 1;
        dat <<= 1;
        if (I2C_SDA) dat |= 0x01;
        DelayUs(I2C_DELAY_US);
    }
    I2C_SCL = 0;
    return dat;
}

/**
 * @brief  发送 ACK 或 NACK
 * @param  ack 0=发送 NACK, 1=发送 ACK
 */
void I2C_SendAck(unsigned char ack)
{
    I2C_SDA = ack ? 1 : 0;
    DelayUs(I2C_DELAY_US);
    I2C_SCL = 1;
    DelayUs(I2C_DELAY_US);
    I2C_SCL = 0;
}

/**
 * @brief  接收 ACK
 * @return 0=收到 ACK, 1=收到 NACK
 */
unsigned char I2C_RecvAck(void)
{
    unsigned char ack;
    I2C_SDA = 1;
    DelayUs(I2C_DELAY_US);
    I2C_SCL = 1;
    DelayUs(I2C_DELAY_US);
    ack = I2C_SDA;
    I2C_SCL = 0;
    return ack;
}
