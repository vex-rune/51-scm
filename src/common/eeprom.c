/*
 * File:    eeprom.c
 * Project: 公共模块 - AT24C02 / M24C02 EEPROM 驱动实现
 * MCU:     STC89C52RC
 * Brief:   基于 i2c.c 实现 5 种标准读写方法
 *
 * 支持的操作:
 *   - Byte Write
 *   - Page Write (最多 8 字节, 起始地址需页对齐)
 *   - Current Address Read
 *   - Random Read
 *   - Sequential Read
 */

#include <stc89c52rc.h>
#include <delay.h>
#include "eeprom.h"

/**
 * @brief  初始化 EEPROM (调用 I2C_Init)
 */
void EEPROM_Init(void)
{
    I2C_Init();
}

/* ============================================================
 * 1. Byte Write - 写一个字节
 *    START -> ADDR+W -> ACK -> WORD ADDR -> ACK -> DATA -> ACK -> STOP
 *    写完后需等待 5ms (写周期)
 * ============================================================ */
void EEPROM_ByteWrite(unsigned char addr, unsigned char dat)
{
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_RecvAck();
    I2C_SendByte(addr);
    I2C_RecvAck();
    I2C_SendByte(dat);
    I2C_RecvAck();
    I2C_Stop();
    DelayMs(EEPROM_WRITE_MS);
}

/* ============================================================
 * 2. Page Write - 一次写一页(最多 8 字节)
 *    起始地址必须页对齐, 否则会发生回卷
 * ============================================================ */
void EEPROM_PageWrite(unsigned char addr, const unsigned char *buf, unsigned char len)
{
    unsigned char i;

    if (len == 0) return;
    if (len > EEPROM_PAGE_SIZE) len = EEPROM_PAGE_SIZE;

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_RecvAck();
    I2C_SendByte(addr);
    I2C_RecvAck();

    for (i = 0; i < len; i++) {
        I2C_SendByte(buf[i]);
        I2C_RecvAck();
    }

    I2C_Stop();
    DelayMs(EEPROM_WRITE_MS);
}

/* ============================================================
 * 3. Current Address Read - 读当前内部地址计数器指向的字节
 * ============================================================ */
unsigned char EEPROM_CurrentAddressRead(void)
{
    unsigned char dat;
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR | 0x01);
    I2C_RecvAck();
    dat = I2C_RecvByte();
    I2C_SendAck(1);  /* NACK */
    I2C_Stop();
    return dat;
}

/* ============================================================
 * 4. Random Read - 读指定地址的一个字节(伪读)
 * ============================================================ */
unsigned char EEPROM_RandomRead(unsigned char addr)
{
    unsigned char dat;

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_RecvAck();
    I2C_SendByte(addr);
    I2C_RecvAck();

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR | 0x01);
    I2C_RecvAck();

    dat = I2C_RecvByte();
    I2C_SendAck(1);  /* NACK */
    I2C_Stop();

    return dat;
}

/* ============================================================
 * 5. Sequential Read - 从指定地址开始连续读 N 个字节
 * ============================================================ */
unsigned char EEPROM_SequentialRead(unsigned char addr, unsigned char *buf, unsigned char len)
{
    unsigned char i;

    if (len == 0) return 0;

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    I2C_RecvAck();
    I2C_SendByte(addr);
    I2C_RecvAck();

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR | 0x01);
    I2C_RecvAck();

    for (i = 0; i < len; i++) {
        buf[i] = I2C_RecvByte();
        I2C_SendAck(i == len - 1 ? 1 : 0);
    }

    I2C_Stop();
    return 1;
}
