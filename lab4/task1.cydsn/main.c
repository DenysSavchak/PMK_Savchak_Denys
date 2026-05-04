#include "project.h"

static uint8_t LED_NUM[] = {
    0xC0,
    0xF9,
    0xA4,
    0xB0,
    0x99,
    0x92,
    0x82,
    0xF8,
    0x80,
    0x90,
    0xBF,
};

static uint8_t keys[4][3];

static void (*col_x_SetDriveMode[3])(uint8_t mode) = {
    col_0_SetDriveMode,
    col_1_SetDriveMode,
    col_2_SetDriveMode
};

static void (*col_x_Write[3])(uint8_t value) = {
    col_0_Write,
    col_1_Write,
    col_2_Write
};

static uint8 (*row_x_Read[4])() = {
    row_0_Read,
    row_1_Read,
    row_2_Read,
    row_3_Read
};

static void FourDigit74HC595_sendData(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (0x80 >> i))
            Pin_DO_Write(1);
        else
            Pin_DO_Write(0);
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot)
{
    if (position >= 8)
    {
        FourDigit74HC595_sendData(0xFF);
        FourDigit74HC595_sendData(0xFF);
        Pin_Latch_Write(1);
        Pin_Latch_Write(0);
        return;
    }
    FourDigit74HC595_sendData(0xFF & ~(1 << position));
    if (dot)
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F);
    else
        FourDigit74HC595_sendData(LED_NUM[digit]);
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

static void initMatrix(void)
{
    for (int c = 0; c < 3; c++)
        col_x_SetDriveMode[c](col_0_DM_DIG_HIZ);
}

static void readMatrix(void)
{
    for (int c = 0; c < 3; c++)
    {
        col_x_SetDriveMode[c](col_0_DM_STRONG);
        col_x_Write[c](0);
        for (int r = 0; r < 4; r++)
            keys[r][c] = row_x_Read[r]();
        col_x_SetDriveMode[c](col_0_DM_DIG_HIZ);
    }
}

int main(void)
{
    CyGlobalIntEnable;
    SW_Tx_UART_Start();
    initMatrix();

    uint8_t last_state = 12;

    for (;;)
    {
        readMatrix();

        if (keys[0][0] == 0 && last_state != 1) {
            last_state = 1;
            FourDigit74HC595_sendOneDigit(0, 1, 0);
            SW_Tx_UART_PutString("Button 1 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[0][0] == 1 && last_state == 1) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 1 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[0][1] == 0 && last_state != 2) {
            last_state = 2;
            FourDigit74HC595_sendOneDigit(0, 2, 0);
            SW_Tx_UART_PutString("Button 2 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[0][1] == 1 && last_state == 2) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 2 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[0][2] == 0 && last_state != 3) {
            last_state = 3;
            FourDigit74HC595_sendOneDigit(0, 3, 0);
            SW_Tx_UART_PutString("Button 3 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[0][2] == 1 && last_state == 3) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 3 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[1][0] == 0 && last_state != 4) {
            last_state = 4;
            FourDigit74HC595_sendOneDigit(0, 4, 0);
            SW_Tx_UART_PutString("Button 4 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[1][0] == 1 && last_state == 4) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 4 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[1][1] == 0 && last_state != 5) {
            last_state = 5;
            FourDigit74HC595_sendOneDigit(0, 5, 0);
            SW_Tx_UART_PutString("Button 5 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[1][1] == 1 && last_state == 5) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 5 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[1][2] == 0 && last_state != 6) {
            last_state = 6;
            FourDigit74HC595_sendOneDigit(0, 6, 0);
            SW_Tx_UART_PutString("Button 6 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[1][2] == 1 && last_state == 6) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 6 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[2][0] == 0 && last_state != 7) {
            last_state = 7;
            FourDigit74HC595_sendOneDigit(0, 7, 0);
            SW_Tx_UART_PutString("Button 7 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[2][0] == 1 && last_state == 7) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 7 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[2][1] == 0 && last_state != 8) {
            last_state = 8;
            FourDigit74HC595_sendOneDigit(0, 8, 0);
            SW_Tx_UART_PutString("Button 8 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[2][1] == 1 && last_state == 8) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 8 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[2][2] == 0 && last_state != 9) {
            last_state = 9;
            FourDigit74HC595_sendOneDigit(0, 9, 0);
            SW_Tx_UART_PutString("Button 9 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[2][2] == 1 && last_state == 9) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 9 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[3][0] == 0 && last_state != 10) {
            last_state = 10;
            FourDigit74HC595_sendOneDigit(0, 0, 1);
            SW_Tx_UART_PutString("Button * pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[3][0] == 1 && last_state == 10) {
            last_state = 12;
            SW_Tx_UART_PutString("Button * released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[3][1] == 0 && last_state != 0) {
            last_state = 0;
            FourDigit74HC595_sendOneDigit(0, 0, 0);
            SW_Tx_UART_PutString("Button 0 pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[3][1] == 1 && last_state == 0) {
            last_state = 12;
            SW_Tx_UART_PutString("Button 0 released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }

        if (keys[3][2] == 0 && last_state != 11) {
            last_state = 11;
            FourDigit74HC595_sendOneDigit(0, 1, 1);
            SW_Tx_UART_PutString("Button # pressed\r\n");
            LED_Write(0); LED2_Write(0); LED3_Write(0);
        }
        if (keys[3][2] == 1 && last_state == 11) {
            last_state = 12;
            SW_Tx_UART_PutString("Button # released\r\n");
            LED_Write(1); LED2_Write(1); LED3_Write(1);
        }
    }
}
