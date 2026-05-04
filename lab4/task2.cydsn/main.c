#include "project.h"

static uint8_t LED_NUM[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
    0xBF, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E, 0x7F
};

void (*write_col[3])(uint8) = {col_0_Write, col_1_Write, col_2_Write};
void (*set_col_dm[3])(uint8) = {col_0_SetDriveMode, col_1_SetDriveMode, col_2_SetDriveMode};
uint8 (*read_row[4])(void) = {row_0_Read, row_1_Read, row_2_Read, row_3_Read};

static void sendData(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Pin_DO_Write((data & (0x80 >> i)) != 0);
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

static void sendOneDigit(uint8_t position, uint8_t digit_idx)
{
    sendData(0xFF & ~(1 << position));
    sendData(LED_NUM[digit_idx]);
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

int main(void)
{
    CyGlobalIntEnable;
    SW_Tx_UART_Start();

    for (int i = 0; i < 3; i++)
        set_col_dm[i](col_0_DM_DIG_HIZ);

    uint8_t position = 0;
    uint8_t mode = 0;
    uint8_t last_key = 255;
    uint8_t last_btn = 1;
    uint32_t btn_hold_ms = 0;
    uint8_t btn_long_triggered = 0;

    SW_Tx_UART_PutString("Mode: DIGIT\r\n");

    for (;;)
    {
        uint8_t btn = Button_Read();
        if (btn == 0)
        {
            btn_hold_ms += 50;
            if (btn_hold_ms >= 2000 && btn_long_triggered == 0)
            {
                btn_long_triggered = 1;
                mode = (mode == 0) ? 1 : 0;
                position = 0;
                SW_Tx_UART_PutString(mode == 0 ? "Mode: DIGIT\r\n" : "Mode: LETTER\r\n");
                LED_Write(mode == 0 ? 0 : 1);
                LED2_Write(mode == 0 ? 1 : 0);
                LED3_Write(1);
                CyDelay(300);
                LED_Write(1);
                LED2_Write(1);
                LED3_Write(1);
            }
        }
        else
        {
            if (btn_hold_ms > 0 && btn_hold_ms < 2000 && last_btn == 0)
            {
                position = (position == 7) ? 0 : (position + 1);
                SW_Tx_UART_PutString("Position: ");
                SW_Tx_UART_PutHexInt(position);
                SW_Tx_UART_PutCRLF();
            }
            btn_hold_ms = 0;
            btn_long_triggered = 0;
        }
        last_btn = btn;

        uint8_t current_key = 255;
        for (int c = 0; c < 3; c++)
        {
            set_col_dm[c](col_0_DM_STRONG);
            write_col[c](0);
            for (int r = 0; r < 4; r++)
            {
                if (read_row[r]() == 0)
                {
                    if (r < 3)
                        current_key = r * 3 + c + 1;
                    else if (c == 0)
                        current_key = 10;
                    else if (c == 1)
                        current_key = 0;
                    else
                        current_key = 11;
                }
            }
            set_col_dm[c](col_0_DM_DIG_HIZ);
        }

        if (current_key != last_key)
        {
            if (current_key != 255)
            {
                SW_Tx_UART_PutString("Key: ");
                SW_Tx_UART_PutHexInt(current_key);
                SW_Tx_UART_PutCRLF();

                if (mode == 0)
                {
                    if (current_key <= 9)
                    {
                        sendOneDigit(position, current_key);
                        LED_Write(current_key == 0 ? 0 : 1);
                        LED2_Write(current_key == 0 ? 1 : 0);
                        LED3_Write(1);
                    }
                    else if (current_key == 10)
                    {
                        sendOneDigit(position, 10);
                        LED_Write(0);
                        LED2_Write(0);
                        LED3_Write(1);
                    }
                    else if (current_key == 11)
                    {
                        sendOneDigit(position, 17);
                        LED_Write(0);
                        LED2_Write(0);
                        LED3_Write(1);
                    }
                }
                else
                {
                    if (current_key >= 1 && current_key <= 6)
                    {
                        sendOneDigit(position, 10 + current_key);
                        LED_Write(1);
                        LED2_Write(1);
                        LED3_Write(0);
                    }
                }
            }
            else
            {
                LED_Write(1);
                LED2_Write(1);
                LED3_Write(1);
                SW_Tx_UART_PutString("Key released\r\n");
            }
            last_key = current_key;
        }
        CyDelay(50);
    }
}