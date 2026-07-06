/*
 * File:    main.c
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 dot matrix picture demo with scrolling text
 *
 * Demo loop:
 *   1. Self test (all on -> all off)
 *   2. Heart
 *   3. Numbers 0~3
 *   4. Smiley
 *   5. Box
 *   6. Scrolling text "Hi! Luke, I am a robot."
 */

#include <matrix.h>
#include <delay.h>

/* ============================================================
 * 8x8 picture data (one byte per row, bit=1 means LED on)
 * ============================================================ */

/* Heart */
static const unsigned char PIC_HEART[8] = {
    0x00, 0x1C, 0x3E, 0x7E, 0x7E, 0x3E, 0x1C, 0x00
};

/* Number 0 */
static const unsigned char PIC_0[8] = {
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x41, 0x3E, 0x00
};

/* Number 1 */
static const unsigned char PIC_1[8] = {
    0x00, 0x42, 0x42, 0x7F, 0x40, 0x40, 0x00, 0x00
};

/* Number 2 */
static const unsigned char PIC_2[8] = {
    0x00, 0x62, 0x51, 0x49, 0x49, 0x49, 0x46, 0x00
};

/* Number 3 */
static const unsigned char PIC_3[8] = {
    0x00, 0x22, 0x41, 0x49, 0x49, 0x49, 0x36, 0x00
};

/* Box (self test pattern) */
static const unsigned char PIC_BOX[8] = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
};

/* Smiley */
static const unsigned char PIC_SMILE[8] = {
    0x00, 0x00, 0x24, 0x00, 0x42, 0x3C, 0x00, 0x00
};

/* ============================================================
 * 8x8 font for "Hi! Luke, I am a robot."
 *   17 chars x 8 bytes = 136 bytes, in ROM (__code)
 *   Format: [c0r0..c0r7, c1r0..c1r7, ...]
 *   Sequence: H, i, !, ' ', L, u, k, e, ',', I, a, m, r, o, b, t, '.'
 * ============================================================ */

static const __code unsigned char FONT_DATA[] = {
    /* 'H' */  0x00, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22,
    /* 'i' */  0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C,
    /* '!' */  0x00, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x08,
    /* ' ' */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 'L' */  0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E,
    /* 'u' */  0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x26, 0x1A,
    /* 'k' */  0x00, 0x20, 0x22, 0x24, 0x38, 0x24, 0x22, 0x22,
    /* 'e' */  0x00, 0x00, 0x1C, 0x22, 0x3E, 0x20, 0x20, 0x1E,
    /* ',' */  0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x10,
    /* 'I' */  0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C,
    /* 'a' */  0x00, 0x00, 0x1C, 0x02, 0x1E, 0x22, 0x22, 0x1E,
    /* 'm' */  0x00, 0x00, 0x36, 0x2A, 0x2A, 0x22, 0x22, 0x22,
    /* 'r' */  0x00, 0x00, 0x3C, 0x22, 0x20, 0x20, 0x20, 0x20,
    /* 'o' */  0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x22, 0x1C,
    /* 'b' */  0x00, 0x20, 0x20, 0x3C, 0x22, 0x22, 0x22, 0x3C,
    /* 't' */  0x00, 0x10, 0x10, 0x3C, 0x10, 0x10, 0x12, 0x0C,
    /* '.' */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08
};

#define FONT_CHAR_COUNT  17

/* ============================================================
 * Main
 * ============================================================ */
void main(void)
{    
    P34 = 1; // led 开
    P35 = 0; // 点阵显示 开
    P36 = 1; // 数码管 关
    Matrix_Init();

    while (1) {
        /* 1. Self test */
        Matrix_SelfTest();
        DelayMs(1000);
        Matrix_Clear();
        DelayMs(500);

        /* 2. Heart for 2s */
        Matrix_Show(PIC_HEART);
        DelayMs(2000);

        /* 3. Numbers 0~3, 1s each */
        Matrix_Show(PIC_0);
        DelayMs(1000);
        Matrix_Show(PIC_1);
        DelayMs(1000);
        Matrix_Show(PIC_2);
        DelayMs(1000);
        Matrix_Show(PIC_3);
        DelayMs(1000);

        /* 4. Smiley for 2s */
        Matrix_Show(PIC_SMILE);
        DelayMs(2000);

        /* 5. Box for 1s */
        Matrix_Show(PIC_BOX);
        DelayMs(1000);

        /* 6. Clear 500ms */
        Matrix_Clear();
        DelayMs(500);

        /* 7. Scrolling text: "Hi! Luke, I am a robot." */
        Matrix_PlayScroll(FONT_DATA, FONT_CHAR_COUNT, 80);

        Matrix_Clear();
        DelayMs(500);
    }
}
