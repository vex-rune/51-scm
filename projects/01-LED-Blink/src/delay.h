/*
 * File:    delay.h
 * Project: 01-LED-Blink
 * Tool:    PlatformIO + SDCC
 * Brief:   延时函数声明
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/**
 * @brief  毫秒级阻塞延时（基于 11.0592 MHz 主频近似校准）
 * @param  ms  延时毫秒数（0 ~ 65535）
 */
void DelayMs(unsigned int ms);

#endif /* __DELAY_H__ */