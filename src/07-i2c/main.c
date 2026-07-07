#include <stc89c52rc.h>
#include <delay.h>

/* —— 主循环 —— */
void main(void) {
    P00 = 0;
    while (1) {
        P00 = !P00;
        DelayMs(10);
    }
}