/* ============================================================
 * 公共模块 - DS18B20 温度传感器驱动
 * MCU:     STC89C52RC
 * Brief:   基于 1-wire 总线的 DS18B20 温度读取
 *
 * 流程:
 *   1) 1-Wire 复位 + 检测应答
 *   2) SKIP ROM (0xCC) - 单从机时跳过 ROM 匹配
 *   3) CONVERT T  (0x44) - 启动温度转换
 *   4) 等待转换完成 (~750ms, 12-bit 精度)
 *   5) 1-Wire 复位
 *   6) SKIP ROM
 *   7) READ SCRATCHPAD (0xBE) - 读 9 字节 (含温度 LSB/MSB/TH/TL/Config/...)
 *   8) 解析温度 (16-bit 符号整数, 单位 1/16 °C)
 *
 * 依赖: onewire.h
 */

#ifndef __DS18B20_H__
#define __DS18B20_H__

/* 缓存最近一次读出的 scratchpad 9 字节 (供调试显示用) */
extern unsigned char DS18B20_Scratchpad[9];

/* 错误码: 0=OK, 1=StartConvert复位失败, 2=ReadScratchpad复位失败, 3=CRC错 */
extern unsigned char DS18B20_ErrCode;

/**
 * @brief  触发一次温度转换 + 读取 scratchpad
 * @param  temp_out  输出: 温度原始值, 单位 0.0625°C (= 1/16 °C)
 *                  正数=零上, 负数=零下
 *                  调用方需要 * 0.0625 才能换算为 °C
 * @return 0  失败 (无应答 / CRC 错), 1  成功
 *
 * @code
 *   signed int raw;
 *   if (DS18B20_GetTemperature(&raw)) {
 *       float celsius = raw * 0.0625;
 *       // 打印: 23.5 °C
 *   }
 * @endcode
 */
unsigned char DS18B20_GetTemperature(signed int *temp_out);

#endif /* __DS18B20_H__ */
