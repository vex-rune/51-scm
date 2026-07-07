/*
 * File:    i2c.h
 * Project: 公共模块 - I2C 软件模拟驱动
 * MCU:     STC89C52RC
 * Brief:   提供 I2C 基础读写原语, 通用从机设备的基类
 *
 * 不在 i2c.c 中硬编码从机地址, 高级 API (eeprom 等) 在自己的 .c 中组合
 */

#ifndef __I2C_H__
#define __I2C_H__

/* I2C 引脚定义 - 在这里修改以适配不同硬件 */
#define I2C_SCL P17
#define I2C_SDA P16

/* I2C 速度控制（延时微秒） */
#define I2C_DELAY_US 5

/* 基础 I2C 原语 (供其他模块组合) */
void I2C_Init(void);                       /* 初始化 I2C GPIO */
void I2C_Start(void);                      /* 发送起始信号 */
void I2C_Stop(void);                       /* 发送停止信号 */
void I2C_SendByte(unsigned char dat);      /* 发送一个字节 */
unsigned char I2C_RecvByte(void);          /* 接收一个字节 */
void I2C_SendAck(unsigned char ack);       /* 发送 ACK/NACK */
unsigned char I2C_RecvAck(void);           /* 接收 ACK */

#endif
