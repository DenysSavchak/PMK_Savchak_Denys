#include "project.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Pin arrays                                                           */
/* col_0 -> P0[0], col_1 -> P0[1], col_2 -> P1[0]                      */
/* row_0 -> P1[1], row_1 -> P1[2], row_2 -> P1[3], row_3 -> P1[4]      */
/* LED=Red(P1[6])  LED2=Green(P0[2])  LED3=Blue(P0[3])                 */
/* ------------------------------------------------------------------ */
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

static uint8_t keys[4][3];

/*
 * Logical key map [row][col]:
 *   1  2  3
 *   4  5  6
 *   7  8  9
 *  10  0 11      (* = 10,  # = 11)
 *
 * '*' (10) = Backspace
 * '#' (11) = Enter / Confirm
 */
static const uint8_t keyMap[4][3] = {
    { 1,  2,  3},
    { 4,  5,  6},
    { 7,  8,  9},
    {10,  0, 11}
};

/* ------------------------------------------------------------------ */
/* Password settings (length > 2 as required)                          */
/* Change PASSWORD[] and PASS_LEN to set your own password             */
/* ------------------------------------------------------------------ */
#define MAX_INPUT_LEN  16

static const uint8_t PASSWORD[] = {1, 2, 3, 4}; /* keys: 1-2-3-4 */
static const uint8_t PASS_LEN   = 4u;

static uint8_t inputBuf[MAX_INPUT_LEN];
static uint8_t inputLen = 0;

/* ------------------------------------------------------------------ */
/* Matrix functions                                                     */
/* ------------------------------------------------------------------ */
static void initMatrix()
{
    for(int i = 0; i < 3; i++)
        col_x_SetDriveMode[i](col_0_DM_DIG_HIZ);
}

static void readMatrix()
{
    for(int col = 0; col < 3; col++)
    {
        col_x_SetDriveMode[col](col_0_DM_STRONG);
        col_x_Write[col](0);
        for(int row = 0; row < 4; row++)
            keys[row][col] = row_x_Read[row]();
        col_x_SetDriveMode[col](col_0_DM_DIG_HIZ);
    }
}

static uint8_t getPressedKey()
{
    for(int row = 0; row < 4; row++)
        for(int col = 0; col < 3; col++)
            if(keys[row][col] == 0)
                return keyMap[row][col];
    return 0xFF;
}

/* ------------------------------------------------------------------ */
/* RGB LED helper (active LOW: 0=ON, 1=OFF)                             */
/* ------------------------------------------------------------------ */
static void setLED(uint8_t r, uint8_t g, uint8_t b)
{
    LED_Write(r);
    LED2_Write(g);
    LED3_Write(b);
}

/* ------------------------------------------------------------------ */
/* Re-print current input line                                          */
/* ------------------------------------------------------------------ */
static void printInputLine()
{
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Input: ");
    for(uint8_t i = 0; i < inputLen; i++)
        SW_Tx_UART_PutString("*");
}

/* ------------------------------------------------------------------ */
/* Check password and show result                                       */
/* ------------------------------------------------------------------ */
static void checkPassword()
{
    uint8_t ok = 1;

    if(inputLen != PASS_LEN)
    {
        ok = 0;
    }
    else
    {
        for(uint8_t i = 0; i < PASS_LEN; i++)
        {
            if(inputBuf[i] != PASSWORD[i])
            {
                ok = 0;
                break;
            }
        }
    }

    if(ok)
    {
        SW_Tx_UART_PutString("Access allowed");
        SW_Tx_UART_PutCRLF();
        setLED(1, 0, 1); /* Green */
        CyDelay(1500);
    }
    else
    {
        SW_Tx_UART_PutString("Access denied");
        SW_Tx_UART_PutCRLF();
        setLED(0, 1, 1); /* Red */
        CyDelay(1500);
    }

    /* Reset and return to idle White */
    inputLen = 0;
    setLED(0, 0, 0); /* White */
    printInputLine();
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Software Transmit UART - Task 3 (Password)");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("* = Backspace  |  # = Confirm");
    SW_Tx_UART_PutCRLF();

    initMatrix();

    /* Idle state -> White */
    setLED(0, 0, 0);

    printInputLine();

    uint8_t last_key     = 0xFF;
    uint8_t key_released = 1;

    for(;;)
    {
        readMatrix();
        uint8_t pressed = getPressedKey();

        if(pressed == 0xFF)
        {
            if(!key_released)
            {
                key_released = 1;
                last_key     = 0xFF;
            }
        }
        else
        {
            if(key_released && pressed != last_key)
            {
                key_released = 0;
                last_key     = pressed;

                switch(pressed)
                {
                    case 11: /* '#' -> confirm */
                        SW_Tx_UART_PutCRLF();
                        checkPassword();
                        break;

                    case 10: /* '*' -> backspace */
                        if(inputLen > 0)
                        {
                            inputLen--;
                            printInputLine();
                        }
                        break;

                    default: /* digit key 0-9 */
                        if(inputLen < MAX_INPUT_LEN)
                        {
                            inputBuf[inputLen++] = pressed;
                            SW_Tx_UART_PutString("*"); /* hide digit */
                        }
                        else
                        {
                            SW_Tx_UART_PutCRLF();
                            SW_Tx_UART_PutString("Buffer full! Press # to confirm or * to clear.");
                            SW_Tx_UART_PutCRLF();
                        }
                        break;
                }
            }
        }
    }
}