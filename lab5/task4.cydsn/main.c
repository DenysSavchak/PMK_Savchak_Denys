#include "project.h"

static const uint8_t SEG[10] = {
    0xC0u, 0xF9u, 0xA4u, 0xB0u, 0x99u,
    0x92u, 0x82u, 0xF8u, 0x80u, 0x90u
};

static uint8_t disp_buf[8];
static uint8_t disp_pos;
static volatile uint32_t ms_cnt;

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
    ms_cnt++;
}

static uint32_t get_ms(void)
{
    uint32_t v;
    uint8_t s = CyEnterCriticalSection();
    v = ms_cnt;
    CyExitCriticalSection(s);
    return v;
}

static void update_display(uint32_t secs)
{
    uint8_t m = (uint8_t)(secs / 60u);
    uint8_t s = (uint8_t)(secs % 60u);
    disp_buf[0] = m / 10u;
    disp_buf[1] = m % 10u;
    disp_buf[2] = s / 10u;
    disp_buf[3] = s % 10u;
    /* позиції 4–7 порожні */
    disp_buf[4] = 0u;
    disp_buf[5] = 0u;
    disp_buf[6] = 0u;
    disp_buf[7] = 0u;
}

int main(void)
{
    CyGlobalIntEnable;
    Timer_Start();
    Timer_int_StartEx(Timer_Int_Handler2);

    uint32_t cnt_secs = 300u;   /* 5 хвилин */
    uint32_t last_sec = get_ms();

    update_display(cnt_secs);

    for(;;)
    {
        uint32_t now = get_ms();
        if (now - last_sec >= 1000u)
        {
            last_sec = now;
            if (cnt_secs > 0u) cnt_secs--;
            update_display(cnt_secs);
        }
    }
}