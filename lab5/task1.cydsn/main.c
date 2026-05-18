#include "project.h"

#define MY_NUMBER   35u

static const uint8_t SEG[10] = {
    0xC0u, 0xF9u, 0xA4u, 0xB0u, 0x99u,
    0x92u, 0x82u, 0xF8u, 0x80u, 0x90u
};

static uint8_t disp_buf[8];
static uint8_t disp_pos;

static void shift_byte(uint8_t data)
{
    for (uint8_t i = 0u; i < 8u; i++) {
        Pin_CLK_Write(0u);
        Pin_DO_Write((data & 0x80u) ? 1u : 0u);
        data <<= 1u;
        Pin_CLK_Write(1u);
    }
}

static void send_digit(uint8_t pos, uint8_t digit)
{
    uint8_t seg   = (digit < 10u) ? SEG[digit] : 0xFFu;
    uint8_t anode = (uint8_t)(~(1u << pos));
    Pin_Latch_Write(0u);
    shift_byte(anode);
    shift_byte(seg);
    Pin_Latch_Write(1u);
}

CY_ISR(Timer_Int_Handler2)
{
    send_digit(disp_pos, disp_buf[disp_pos]);
    disp_pos = (disp_pos + 1u) & 7u;
}

int main(void)
{
    CyGlobalIntEnable;
    Timer_Start();
    Timer_int_StartEx(Timer_Int_Handler2);

    uint32_t num = MY_NUMBER;
    for (int8_t i = 7; i >= 0; i--) {
        disp_buf[i] = (uint8_t)(num % 10u);
        num /= 10u;
    }

    for(;;) {}
}