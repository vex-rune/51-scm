/*
 * File:    main.c
 * Project: 12-all-in-one
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   综合演示项目 - OLED版
 *
 * 功能模块:
 *   - LED流水灯 (P0口)
 *   - 8位数码管显示 (P0口)
 *   - 8x8点阵显示 (P0口)
 *   - OLED显示 + DS1302时钟
 *
 * 按键控制:
 *   P42 - LED模式
 *   P43 - 数码管模式
 *   P32 - 点阵模式
 */

#include <stc89c52rc.h>
#include <delay.h>
#include <smg.h>
#include <matrix.h>
#include <oled.h>
#include <ds1302.h>
#include <timer.h>

/* 按键定义 */
#define KEY_LED     P42
#define KEY_SMG     P43
#define KEY_MATRIX  P32

/* 设备类型 */
typedef enum { DEV_NONE = 0, DEV_LED, DEV_SMG, DEV_MATRIX } DeviceType;

/* 全局状态 */
static DeviceType g_current_dev = DEV_NONE;
static unsigned char g_led_running = 0;
static unsigned char g_smgrunning = 0;
static unsigned char g_matrix_running = 0;
static unsigned char key_led_state = 0;
static unsigned char key_smg_state = 0;
static unsigned char key_matrix_state = 0;

/* 互斥控制 */
unsigned char g_smg_enabled = 0;
unsigned char g_matrix_enabled = 0;

/* LED状态 */
static unsigned char led_mask = 0xFE;

/* 点阵图案 */
static unsigned char current_pic = 0;
static unsigned int switch_counter = 0;

/* DS1302时间 */
static DS1302_Time g_time;
static unsigned char g_time_valid = 0;

/* 点阵图案 (Flash) */
static const unsigned char __code PIC_HEART[8] = {0x18,0x3C,0x7E,0xFF,0xFF,0x7E,0x3C,0x18};
static const unsigned char __code PIC_SMILE[8] = {0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C};
static const unsigned char __code PIC_0[8] = {0x3C,0x66,0x6E,0x76,0x76,0x6E,0x66,0x3C};
static const unsigned char __code PIC_1[8] = {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00};
static const unsigned char __code * __code PICS[] = {PIC_HEART, PIC_SMILE, PIC_0, PIC_1};

/* P0设备控制 */
static void P0_Device_Off(void)
{
    g_smg_enabled = 0;
    g_matrix_enabled = 0;
    P34 = 0; P35 = 1; P36 = 0;
    P0 = 0xFF;
}

static void Switch_To_LED(void)
{
    g_smg_enabled = 0; g_matrix_enabled = 0;
    DelayMs(1);
    P0_Device_Off(); DelayMs(1);
    P34 = 1; P35 = 1; P36 = 0;
    g_current_dev = DEV_LED;
}

static void Switch_To_SMG(void)
{
    g_matrix_enabled = 0;
    DelayMs(1);
    P0_Device_Off(); DelayMs(1);
    P34 = 0; P35 = 1; P36 = 1;
    g_smg_enabled = 1;
    g_current_dev = DEV_SMG;
}

static void Switch_To_Matrix(void)
{
    g_smg_enabled = 0;
    DelayMs(1);
    P0_Device_Off(); DelayMs(1);
    P34 = 0; P35 = 0; P36 = 0;
    g_matrix_enabled = 1;
    g_current_dev = DEV_MATRIX;
}

/* 点阵扫描 */
static void Task_Matrix(void)
{
    if (!g_matrix_running) return;
    if (g_current_dev == DEV_MATRIX) Matrix_Scan();
}

/* 点阵图案切换 (2秒) */
static void Task_Matrix_Switch(void)
{
    if (!g_matrix_running) return;
    if (++switch_counter >= 8000) {
        switch_counter = 0;
        if (++current_pic >= 4) current_pic = 0;
        Matrix_SetBufferCode(PICS[current_pic]);
    }
}

/* OLED显示状态 */
static void OLED_ShowStatus(void)
{
    char buf[12];

    // 日期时间
    if (g_time_valid) {
        buf[0]='2'; buf[1]='0';
        buf[2]=(g_time.year>>4)+'0'; buf[3]=(g_time.year&0x0F)+'0'; buf[4]='-';
        buf[5]=(g_time.month>>4)+'0'; buf[6]=(g_time.month&0x0F)+'0'; buf[7]='-';
        buf[8]=(g_time.day>>4)+'0'; buf[9]=(g_time.day&0x0F)+'0'; buf[10]=0;
        OLED_ShowString(0, 0, buf);

        buf[0]=(g_time.hour>>4)+'0'; buf[1]=(g_time.hour&0x0F)+'0'; buf[2]=':';
        buf[3]=(g_time.minute>>4)+'0'; buf[4]=(g_time.minute&0x0F)+'0'; buf[5]=':';
        buf[6]=(g_time.second>>4)+'0'; buf[7]=(g_time.second&0x0F)+'0'; buf[8]=0;
        OLED_ShowString(2, 0, buf);
    } else {
        OLED_ShowString(0, 0, "Time: N/A");
    }

    // 当前模式
    OLED_ShowString(4, 0, "Mode:");
    if (g_current_dev == DEV_LED) OLED_ShowString(4, 48, "LED");
    else if (g_current_dev == DEV_SMG) OLED_ShowString(4, 48, "7-Seg");
    else if (g_current_dev == DEV_MATRIX) OLED_ShowString(4, 48, "Matrix");
    else OLED_ShowString(4, 48, "None");

    // 运行状态
    OLED_ShowString(6, 0, "Status:");
    if ((g_current_dev==DEV_LED && g_led_running) ||
        (g_current_dev==DEV_SMG && g_smgrunning) ||
        (g_current_dev==DEV_MATRIX && g_matrix_running))
        OLED_ShowString(6, 64, "RUN");
    else
        OLED_ShowString(6, 64, "STOP");
}

/* 时间更新 (1秒) */
static void Task_TimeUpdate(void)
{
    static unsigned int cnt = 0;
    if (++cnt >= 4000) {
        cnt = 0;
        if (DS1302_ReadTime(&g_time)) g_time_valid = 1;
    }
}

/* OLED刷新 (1秒) */
static void Task_OLED_Refresh(void)
{
    static unsigned int cnt = 0;
    if (++cnt >= 4000) { cnt = 0; OLED_ShowStatus(); }
}

/* 按键扫描 */
static void Task_KeyScan(void)
{
    static unsigned char prev_led = 1, prev_smg = 1, prev_matrix = 1;

    unsigned char curr_led = KEY_LED;
    if (prev_led == 1 && curr_led == 0) key_led_state = 1;
    prev_led = curr_led;

    unsigned char curr_smg = KEY_SMG;
    if (prev_smg == 1 && curr_smg == 0) key_smg_state = 1;
    prev_smg = curr_smg;

    unsigned char curr_matrix = KEY_MATRIX;
    if (prev_matrix == 1 && curr_matrix == 0) key_matrix_state = 1;
    prev_matrix = curr_matrix;
}

void main(void)
{
    P0_Device_Off();

    Matrix_Init();
    Smg_Init();
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "All-in-One");

    DS1302_Init();
    DS1302_Start();

    Timer_Init();
    Timer_Register(Smg_Scan);
    Timer_Register(Task_Matrix);
    Timer_Register(Task_Matrix_Switch);
    Timer_Register(Task_TimeUpdate);
    Timer_Register(Task_OLED_Refresh);

    while (1) {
        /* 按键扫描 (5ms) */
        static unsigned int key_cnt = 0;
        if (++key_cnt >= 20) { key_cnt = 0; Task_KeyScan(); }

        /* LED流水灯 (300ms) */
        static unsigned int led_cnt = 0;
        if (g_led_running && ++led_cnt >= 1200) {
            led_cnt = 0;
            if (g_current_dev != DEV_LED) Switch_To_LED();
            P0 = led_mask;
            led_mask = (led_mask << 1) | 0x01;
            if (led_mask == 0xFF) led_mask = 0xFE;
        }

        /* 处理按键 */
        if (key_led_state) {
            key_led_state = 0;
            g_led_running = !g_led_running;
            if (g_led_running) { led_mask = 0xFE; }
            else { P0_Device_Off(); }
        }
        if (key_smg_state) {
            key_smg_state = 0;
            g_smgrunning = !g_smgrunning;
            if (g_smgrunning) { Switch_To_SMG(); Smg_UpdateBuf("12345678"); }
            else { P0_Device_Off(); }
        }
        if (key_matrix_state) {
            key_matrix_state = 0;
            g_matrix_running = !g_matrix_running;
            if (g_matrix_running) { Switch_To_Matrix(); Matrix_SetBufferCode(PICS[0]); }
            else { P0_Device_Off(); }
        }
    }
}
