#include "project.h"

static uint8_t LED_NUM[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90
};

#define SEG_OFF  0xFF
#define SEG_S    0x92
#define SEG_U    0xC1
#define SEG_C    0xC6
#define SEG_E    0x86
#define SEG_d    0xA1
#define SEG_n    0xAB
#define SEG_I    0xF9

#define STATE_INPUT    0
#define STATE_SUCCESS  1
#define STATE_DENIED   2
#define STATE_REINPUT  3

static const uint8_t PASSWORD[4] = {1, 2, 3, 4};

/* SUCCESS = 7 символів, DENIED = 6 символів */
static const uint8_t SUCCESS_MSG[7] = {SEG_S, SEG_U, SEG_C, SEG_C, SEG_E, SEG_S, SEG_S};
static const uint8_t DENIED_MSG[6]  = {SEG_d, SEG_E, SEG_n, SEG_I, SEG_E, SEG_d};

static volatile uint8_t disp_buf[8];
static volatile uint8_t led_counter = 0;

static volatile uint8_t  timer_minutes = 2;
static volatile uint8_t  timer_seconds = 0;
static volatile uint32_t timer_ms      = 0;
static volatile uint8_t  timer_active  = 0;

static volatile uint32_t scroll_tick = 0;
static volatile uint8_t  scroll_pos  = 0;

static uint8_t state       = STATE_INPUT;
static uint8_t entered[4];
static uint8_t input_pos   = 0;
static uint8_t reinput_pos = 0;

static void (*COLUMN_x_SetDriveMode[3])(uint8_t) = {
    col_0_SetDriveMode, col_1_SetDriveMode, col_2_SetDriveMode
};
static void (*COLUMN_x_Write[3])(uint8_t) = {
    col_0_Write, col_1_Write, col_2_Write
};
static uint8 (*ROW_x_Read[4])(void) = {
    row_0_Read, row_1_Read, row_2_Read, row_3_Read
};

static const int8_t KEYMAP[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {-1, 0, -2}
};

/* ===== 74HC595 ===== */

static void FourDigit74HC595_sendData(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Pin_DO_Write((data & (0x80 >> i)) ? 1 : 0);
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t segment_code)
{
    FourDigit74HC595_sendData(0xFF & ~(1 << position));
    FourDigit74HC595_sendData(segment_code);
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

/* ===== Дисплей ===== */

static void clearAll(void)
{
    for (uint8_t i = 0; i < 8; i++)
        disp_buf[i] = SEG_OFF;
}

static void updateTimer(void)
{
    /* Таймер — лише на позиціях 0–3 (під час STATE_REINPUT) */
    disp_buf[0] = LED_NUM[timer_minutes / 10];
    disp_buf[1] = LED_NUM[timer_minutes % 10] & 0x7F;
    disp_buf[2] = LED_NUM[timer_seconds / 10];
    disp_buf[3] = LED_NUM[timer_seconds % 10];
}

/* Прокрутка SUCCESS по всіх 8 розрядах */
static void updateSuccessScroll(void)
{
    for (uint8_t i = 0; i < 8; i++)
        disp_buf[i] = SUCCESS_MSG[(scroll_pos + i) % 7];
}

/* Прокрутка DENIED по всіх 8 розрядах */
static void updateDeniedScroll(void)
{
    for (uint8_t i = 0; i < 8; i++)
        disp_buf[i] = DENIED_MSG[(scroll_pos + i) % 6];
}

/* ===== ISR ===== */

CY_ISR(Timer_Int_Handler2)
{
    /* Відлік таймера блокування */
    if (timer_active)
    {
        timer_ms++;
        if (timer_ms >= 1000)
        {
            timer_ms = 0;
            if (timer_minutes == 0 && timer_seconds == 0)
            {
                timer_active = 0;
            }
            else
            {
                if (timer_seconds == 0) { timer_minutes--; timer_seconds = 59; }
                else                      timer_seconds--;
            }
            /* Таймер відображаємо тільки у STATE_REINPUT */
            if (state == STATE_REINPUT)
                updateTimer();
        }
    }

    /* Прокрутка напису */
    scroll_tick++;
    if (scroll_tick >= 400)
    {
        scroll_tick = 0;
        scroll_pos++;
        if (state == STATE_SUCCESS)
        {
            if (scroll_pos >= 7) scroll_pos = 0;
            updateSuccessScroll();
        }
        else if (state == STATE_DENIED)
        {
            if (scroll_pos >= 6) scroll_pos = 0;
            updateDeniedScroll();
        }
    }

    FourDigit74HC595_sendOneDigit(led_counter, disp_buf[led_counter]);
    if (++led_counter >= 8) led_counter = 0;
}

/* ===== Клавіатура ===== */

static int8_t scanKeypad(void)
{
    for (int c = 0; c < 3; c++)
    {
        COLUMN_x_SetDriveMode[c](col_0_DM_STRONG);
        COLUMN_x_Write[c](0);
        for (int r = 0; r < 4; r++)
        {
            if (ROW_x_Read[r]() == 0)
            {
                COLUMN_x_SetDriveMode[c](col_0_DM_DIG_HIZ);
                return KEYMAP[r][c];
            }
        }
        COLUMN_x_SetDriveMode[c](col_0_DM_DIG_HIZ);
    }
    return -3;
}

/* ===== Логіка стану ===== */

static uint8_t checkPassword(void)
{
    for (int i = 0; i < 4; i++)
        if (entered[i] != PASSWORD[i]) return 0;
    return 1;
}

static void goSuccess(void)
{
    state       = STATE_SUCCESS;
    timer_active = 0;
    scroll_pos  = 0;
    scroll_tick = 0;
    updateSuccessScroll();   /* всі 8 розрядів */
}

static void goDenied(void)
{
    state       = STATE_DENIED;
    scroll_pos  = 0;
    scroll_tick = 0;
    updateDeniedScroll();    /* всі 8 розрядів */
}

/* ===== main ===== */

int main(void)
{
    CyGlobalIntEnable;

    for (int c = 0; c < 3; c++)
        COLUMN_x_SetDriveMode[c](col_0_DM_DIG_HIZ);

    clearAll();

    Timer_int_StartEx(Timer_Int_Handler2);
    Timer_Start();

    int8_t last_key = -3;

    for (;;)
    {
        int8_t key = scanKeypad();

        if (key != last_key && key >= 0 && key <= 9)
        {
            /* --- STATE_INPUT: вводимо пароль на позиції 0–3 --- */
            if (state == STATE_INPUT)
            {
                if (input_pos < 4)
                {
                    entered[input_pos]  = key;
                    disp_buf[input_pos] = LED_NUM[key];
                    input_pos++;

                    if (input_pos == 4)
                    {
                        if (checkPassword())
                        {
                            goSuccess();
                        }
                        else
                        {
                            timer_minutes = 2;
                            timer_seconds = 0;
                            timer_ms      = 0;
                            timer_active  = 1;
                            goDenied();
                        }
                        input_pos = 0;
                    }
                }
            }
            /* --- STATE_DENIED: перший натиск починає повторний ввід --- */
            else if (state == STATE_DENIED)
            {
                state       = STATE_REINPUT;
                reinput_pos = 0;
                /* 0–3: таймер, 4–7: цифри нового вводу */
                updateTimer();
                disp_buf[4] = LED_NUM[key];
                disp_buf[5] = SEG_OFF;
                disp_buf[6] = SEG_OFF;
                disp_buf[7] = SEG_OFF;
                entered[reinput_pos] = key;
                reinput_pos = 1;
            }
            /* --- STATE_REINPUT: продовжуємо вводити на позиціях 4–7 --- */
            else if (state == STATE_REINPUT)
            {
                if (reinput_pos < 4)
                {
                    entered[reinput_pos]      = key;
                    disp_buf[4 + reinput_pos] = LED_NUM[key];
                    reinput_pos++;

                    if (reinput_pos == 4)
                    {
                        if (checkPassword())
                            goSuccess();
                        else
                            goDenied();
                        reinput_pos = 0;
                    }
                }
            }
        }

        last_key = key;
        CyDelay(20);
    }
}