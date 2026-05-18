#include "project.h"

/*
 * Варіант 9: «Кожен другий на 2 позиції вправо»
 * Парні позиції  (0, 2, 4, 6) — зсуваються на 1 за тік (300 мс)
 * Непарні позиції (1, 3, 5, 7) — зсуваються на 2 за тік (300 мс)
 */

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

int main(void)
{
    CyGlobalIntEnable;
    Timer_Start();
    Timer_int_StartEx(Timer_Int_Handler2);

    static const uint8_t seq[8] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    uint8_t  even_off  = 0u;
    uint8_t  odd_off   = 0u;
    uint32_t last_tick = get_ms();

    for(;;)
    {
        uint32_t now = get_ms();
        if (now - last_tick >= 300u)
        {
            last_tick = now;

            even_off = (even_off + 1u) % 8u;
            odd_off  = (odd_off  + 2u) % 8u;

            /* Парні позиції: +1 за тік */
            disp_buf[0] = seq[even_off];
            disp_buf[2] = seq[(even_off + 2u) % 8u];
            disp_buf[4] = seq[(even_off + 4u) % 8u];
            disp_buf[6] = seq[(even_off + 6u) % 8u];

            /* Непарні позиції: +2 за тік */
            disp_buf[1] = seq[(odd_off + 1u) % 8u];
            disp_buf[3] = seq[(odd_off + 3u) % 8u];
            disp_buf[5] = seq[(odd_off + 5u) % 8u];
            disp_buf[7] = seq[(odd_off + 7u) % 8u];
        }
    }
}