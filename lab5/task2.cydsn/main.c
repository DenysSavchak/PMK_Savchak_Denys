#include "project.h"

#define PWD_LEN  4

static const uint8_t SEG[10] = {
    0xC0u, 0xF9u, 0xA4u, 0xB0u, 0x99u,
    0x92u, 0x82u, 0xF8u, 0x80u, 0x90u
};

static const uint8_t password[PWD_LEN] = {1, 2, 3, 4};
static uint8_t input_buf[PWD_LEN];

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

static int8_t scan_once(void)
{
    static const uint8_t map[4][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
        {10, 0, 11}
    };
    for (uint8_t c = 0u; c < 3u; c++) {
        col_0_Write(c == 0u ? 0u : 1u);
        col_1_Write(c == 1u ? 0u : 1u);
        col_2_Write(c == 2u ? 0u : 1u);
        CyDelayUs(50u);
        uint8_t rows[4] = {
            row_0_Read(), row_1_Read(), row_2_Read(), row_3_Read()
        };
        col_0_Write(1u); col_1_Write(1u); col_2_Write(1u);
        for (uint8_t r = 0u; r < 4u; r++)
            if (rows[r] == 0u) return (int8_t)map[r][c];
    }
    return -1;
}

static int8_t wait_key(void)
{
    int8_t key;
    do { key = scan_once(); } while (key < 0);
    CyDelay(30u);
    while (scan_once() >= 0) {}
    CyDelay(20u);
    return key;
}

static void blink_all(uint8_t digit, uint8_t times)
{
    for (uint8_t b = 0u; b < times; b++) {
        for (uint8_t i = 0u; i < 8u; i++) disp_buf[i] = digit;
        CyDelay(400u);
        for (uint8_t i = 0u; i < 8u; i++) disp_buf[i] = 0u;
        CyDelay(400u);
    }
}

int main(void)
{
    CyGlobalIntEnable;
    Timer_Start();
    Timer_int_StartEx(Timer_Int_Handler2);
    col_0_Write(1u); col_1_Write(1u); col_2_Write(1u);

    for (uint8_t i = 0u; i < 8u; i++) disp_buf[i] = 0u;

    for(;;)
    {
        /* Чекаємо натискання '#' */
        int8_t k = wait_key();
        if (k != 11) continue;

        /* Очищаємо дисплей, збираємо 4 цифри */
        for (uint8_t i = 0u; i < 8u; i++) disp_buf[i] = 0u;
        uint8_t len = 0u;

        while (len < PWD_LEN)
        {
            int8_t key = wait_key();
            if (key < 0 || key > 9) continue;
            input_buf[len]  = (uint8_t)key;
            disp_buf[len]   = (uint8_t)key;
            len++;
        }

        CyDelay(300u);

        /* Перевірка пароля */
        uint8_t ok = 1u;
        for (uint8_t i = 0u; i < PWD_LEN; i++)
            if (input_buf[i] != password[i]) { ok = 0u; break; }

        /* Вірно → мигає 8, невірно → мигає 1 */
        blink_all(ok ? 8u : 1u, 4u);

        for (uint8_t i = 0u; i < 8u; i++) disp_buf[i] = 0u;
    }
}