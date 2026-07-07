/*
 * File:    eeprom.h
 * Project: 公共模块 - AT24C02 / M24C02 EEPROM 驱动
 * MCU:     STC89C52RC
 * Brief:   基于 i2c.c 的高层 EEPROM 驱动, 实现 5 种标准操作
 *
 * 依赖: i2c.c, delay.c
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "i2c.h"

/* AT24C02 地址（7位格式） */
#ifndef EEPROM_ADDR
#define EEPROM_ADDR 0xA0
#endif

/* AT24C02: 256 字节 = 16 页 × 16 字节 */
#define EEPROM_PAGE_SIZE  16

/* 写周期等待时间 (ms) */
#define EEPROM_WRITE_MS   5

/* 初始化 */
void EEPROM_Init(void);

/* 写操作 */
void EEPROM_ByteWrite(unsigned char addr, unsigned char dat);
void EEPROM_PageWrite(unsigned char addr, const unsigned char *buf, unsigned char len);

/* 读操作 */
unsigned char EEPROM_CurrentAddressRead(void);
unsigned char EEPROM_RandomRead(unsigned char addr);
unsigned char EEPROM_SequentialRead(unsigned char addr, unsigned char *buf, unsigned char len);

#endif
