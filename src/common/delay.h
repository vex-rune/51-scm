/*
 * File:    delay.h
 * Project: 公共模块
 * Tool:    PlatformIO + SDCC
 * Brief:   毫秒级阻塞延时函数声明（基于 11.0592 MHz 主频校准）
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/**
 * @brief  毫秒级阻塞延时（基于 11.0592 MHz 主频近似校准）
 * @param  ms  延时毫秒数（0 ~ 65535）
 */
void DelayMs(unsigned int ms);

/**
 * @brief  微秒级阻塞延时（基于 11.0592 MHz 主频校准）
 * @param  us  延时微秒数（约 0 ~ 255）
 */
void DelayUs(unsigned char us);

#endif /* __DELAY_H__ */
