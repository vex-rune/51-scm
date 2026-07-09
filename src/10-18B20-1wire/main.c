/*
 * File:    main.c
 * Project: 10-18B20-1wire
 * MCU:     STC89C52RC
 * Brief:   DS18B20 测试 - 手动拉低测试
 */
#include <stc89c52rc.h>
#include <delay.h>
#include <oled.h>

/* 使用 P3.7 测试 (标准 8051 端口) */
#define DQ  P37

int main(void)
{
    unsigned char test1, test2, test3;
    
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "DS18B20 Test");
    
    while (1) {
        /* 测试 1: DQ 设为 1，读取 */
        DQ = 1;
        DelayMs(10);
        test1 = DQ;
        
        /* 测试 2: DQ 设为 0，读取 */
        DQ = 0;
        DelayMs(10);
        test2 = DQ;
        
        /* 测试 3: DQ 释放(1)，等待，读取 */
        DQ = 1;
        DelayMs(10);
        test3 = DQ;
        
        /* 显示结果 */
        OLED_ShowString(2, 0, "DQ=1 read: ");
        OLED_ShowAsciiAt(2, 11*8, test1 ? '1' : '0');
        
        OLED_ShowString(4, 0, "DQ=0 read: ");
        OLED_ShowAsciiAt(4, 11*8, test2 ? '1' : '0');
        
        OLED_ShowString(6, 0, "DQ=1 read: ");
        OLED_ShowAsciiAt(6, 11*8, test3 ? '1' : '0');
        
        /* 提示 */
        if (test1 == 1 && test2 == 0 && test3 == 1) {
            OLED_ShowString(7, 0, "P44 OK!");
        } else {
            OLED_ShowString(7, 0, "P44 ERROR!");
        }
        
        DelayMs(500);
    }
}
