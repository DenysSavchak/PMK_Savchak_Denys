#include "project.h"

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
 */
static const uint8_t keyMap[4][3] = {
    { 1,  2,  3},
    { 4,  5,  6},
    { 7,  8,  9},
    {10,  0, 11}
};

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

static void printMatrix()
{
    SW_Tx_UART_PutCRLF();
    for(int row = 0; row < 4; row++)
    {
        for(int col = 0; col < 3; col++)
        {
            SW_Tx_UART_PutHexInt(keys[row][col]);
            SW_Tx_UART_PutString(" ");
        }
        SW_Tx_UART_PutCRLF();
    }
    SW_Tx_UART_PutCRLF();
}

/* Returns logical key number of first pressed key, 0xFF if none */
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
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Software Transmit UART - Task 2 (switch-case)");
    SW_Tx_UART_PutCRLF();

    initMatrix();

    /* Initially: White */
    setLED(0, 0, 0);

    uint8_t last_key     = 0xFF;
    uint8_t key_released = 1;

    for(;;)
    {
        readMatrix();
        uint8_t pressed = getPressedKey();

        if(pressed == 0xFF)
        {
            /* No key pressed */
            if(!key_released)
            {
                key_released = 1;
                last_key     = 0xFF;
                SW_Tx_UART_PutString("Button released");
                SW_Tx_UART_PutCRLF();
                setLED(1, 1, 1); /* Black */
                setLED(0, 0, 0); /* White */
            }
        }
        else
        {
            if(key_released || pressed != last_key)
            {
                key_released = 0;
                last_key     = pressed;

                /* Print button label */
                SW_Tx_UART_PutString("Button ");
                switch(pressed)
                {
                    case 10: SW_Tx_UART_PutString("*"); break;
                    case 11: SW_Tx_UART_PutString("#"); break;
                    default:
                    {
                        char buf[2] = { '0' + pressed, '\0' };
                        SW_Tx_UART_PutString(buf);
                        break;
                    }
                }
                SW_Tx_UART_PutString(" pressed -> ");

                /*
                 * Color table (Table 1):
                 *   Red    -> 1, 7
                 *   Green  -> 2, 8
                 *   Blue   -> 3, 9
                 *   Yellow -> 4, *(10)
                 *   Purple -> 5, 0
                 *   Cyan   -> 6, #(11)
                 */
                switch(pressed)
                {
                    case 1:
                    case 7:
                        SW_Tx_UART_PutString("Red");
                        setLED(0, 1, 1);
                        break;

                    case 2:
                    case 8:
                        SW_Tx_UART_PutString("Green");
                        setLED(1, 0, 1);
                        break;

                    case 3:
                    case 9:
                        SW_Tx_UART_PutString("Blue");
                        setLED(1, 1, 0);
                        break;

                    case 4:
                    case 10: /* * */
                        SW_Tx_UART_PutString("Yellow");
                        setLED(0, 0, 1);
                        break;

                    case 5:
                    case 0:
                        SW_Tx_UART_PutString("Purple");
                        setLED(0, 1, 0);
                        break;

                    case 6:
                    case 11: /* # */
                        SW_Tx_UART_PutString("Cyan");
                        setLED(1, 0, 0);
                        break;

                    default:
                        SW_Tx_UART_PutString("Unknown");
                        break;
                }

                SW_Tx_UART_PutCRLF();
                printMatrix();
            }
        }
    }
}