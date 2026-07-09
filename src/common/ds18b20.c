/*
 * File:    ds18b20.c
 * Project: 公共模块 - DS18B20 温度传感器驱动实现
 * MCU:     STC89C52RC
 * Brief:   基于 1-wire 总线读取 DS18B20 温度
 *
 * 依赖: delay.h, onewire.h
 */

#include <stc89c52rc.h>
#include <delay.h>
#include "onewire.h"
#include "ds18b20.h"

/* DS18B20 ROM/Function 命令 */
#define DS18B20_CMD_SKIP_ROM         0xCC
#define DS18B20_CMD_CONVERT_T        0x44
#define DS18B20_CMD_READ_SCRATCHPAD  0xBE

/* 12-bit 精度转换时间 (75ms/9-bit, 187.5ms/10-bit, 375ms/11-bit, 750ms/12-bit) */
#define DS18B20_CONVERT_MS  750

/* 缓存最近一次读出的 scratchpad 9 字节 (供调试显示用) */
unsigned char DS18B20_Scratchpad[9];

/* 错误码 */
unsigned char DS18B20_ErrCode = 0;

/**
 * @brief  DS18B20 CRC8 校验 (poly = 0x8C, 即 X8 + X5 + X4 + 1)
 * @param  data  8 字节数据 (scratchpad 前 8 字节)
 * @param  crc   从机返回的 CRC 字节 (data[8])
 * @return 1 CRC 正确, 0 CRC 错误
 */
static unsigned char DS18B20_CheckCRC(const unsigned char *data, unsigned char crc_in)
{
    unsigned char crc = 0x00;
    unsigned char i, j;

    for (i = 0; i < 8; i++) {
        crc = crc ^ data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc >>= 1;
            }
        }
    }
    return (crc == crc_in) ? 1 : 0;
}

/**
 * @brief  触发 DS18B20 温度转换
 * @return 0  失败 (无应答), 1  成功
 */
static unsigned char DS18B20_StartConvert(void)
{
    if (!OneWire_Reset()) {
        return 0;
    }
    OneWire_WriteByte(DS18B20_CMD_SKIP_ROM);
    OneWire_WriteByte(DS18B20_CMD_CONVERT_T);
    return 1;
}

/**
 * @brief  读 DS18B20 scratchpad 9 字节
 * @param  buf  至少 9 字节的接收缓冲区
 * @return 0  失败, 1  成功
 */
static unsigned char DS18B20_ReadScratchpad(unsigned char *buf)
{
    unsigned char i;

    if (!OneWire_Reset()) {
        return 0;
    }
    OneWire_WriteByte(DS18B20_CMD_SKIP_ROM);
    OneWire_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);

    for (i = 0; i < 9; i++) {
        buf[i] = OneWire_ReadByte();
    }
    return 1;
}

/**
 * @brief  一次完整温度读取 (转换 + 读 scratchpad + 解析)
 * @param  temp_out  输出 16-bit 温度原始值 (1/16 °C 单位)
 * @return 0  失败, 1  成功
 */
unsigned char DS18B20_GetTemperature(signed int *temp_out)
{
    signed int raw;

    DS18B20_ErrCode = 0;

    if (!DS18B20_StartConvert()) {
        DS18B20_ErrCode = 1;  /* 发起转换复位失败 */
        return 0;
    }
    DelayMs(DS18B20_CONVERT_MS);

    if (!DS18B20_ReadScratchpad(DS18B20_Scratchpad)) {
        DS18B20_ErrCode = 2;  /* 读 scratchpad 复位失败 */
        return 0;
    }

    /* CRC 校验 - 如果不对则丢弃 (数据可能在采样窗口边缘出错) */
    if (!DS18B20_CheckCRC(&DS18B20_Scratchpad[0], DS18B20_Scratchpad[8])) {
        DS18B20_ErrCode = 3;  /* CRC 错 */
        return 0;
    }

    /* 温度 = (MSB << 8) | LSB, 16-bit 有符号, 单位 1/16 °C */
    raw = ((signed int)(signed char)DS18B20_Scratchpad[1] << 8) | DS18B20_Scratchpad[0];

    *temp_out = raw;
    return 1;
}
