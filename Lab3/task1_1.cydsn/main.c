#include "project.h"

/* ------------------------------------------------------------------ */
/* Arrays of pointers to pin functions                                  */
/* col_0 -> P0[0], col_1 -> P0[1], col_2 -> P1[0]                      */
/* row_0 -> P1[1], row_1 -> P1[2], row_2 -> P1[3], row_3 -> P1[4]      */
/* LED   -> P1[6] (Red)                                                 */
/* LED2  -> P0[2] (Green)                                               */
/* LED3  -> P0[3] (Blue)                                                */
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

/* Raw key state matrix (0 = pressed, 1 = released) */
static uint8_t keys[4][3];

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

static uint8_t anyKeyPressed()
{
    for(int r = 0; r < 4; r++)
        for(int c = 0; c < 3; c++)
            if(keys[r][c] == 0) return 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* RGB LED helper (active LOW: 0=ON, 1=OFF)                             */
/* LED=Red, LED2=Green, LED3=Blue                                       */
/* Colors:                                                              */
/*   White  = 0,0,0   Red    = 0,1,1   Green  = 1,0,1                  */
/*   Blue   = 1,1,0   Yellow = 0,0,1   Purple = 0,1,0   Cyan = 1,0,0   */
/*   Black  = 1,1,1                                                     */
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
    SW_Tx_UART_PutString("Software Transmit UART - Task 1");
    SW_Tx_UART_PutCRLF();

    initMatrix();

    /* Initially: all buttons released -> White */
    setLED(0, 0, 0);

    uint8_t last_state   = 0xFF;
    uint8_t key_released = 1;

    for(;;)
    {
        readMatrix();

        if(!anyKeyPressed())
        {
            /* ---- all keys released ---- */
            if(!key_released)
            {
                key_released = 1;
                last_state   = 0xFF;
                SW_Tx_UART_PutString("Button released");
                SW_Tx_UART_PutCRLF();
                setLED(1, 1, 1); /* Black (momentary) */
                setLED(0, 0, 0); /* back to White     */
            }
        }
        else
        {
            /* ---- at least one key pressed ---- */
            if(key_released)
            {
                key_released = 0;

                /*
                 * Physical layout [row][col]:
                 *   [0][0]=1  [0][1]=2  [0][2]=3
                 *   [1][0]=4  [1][1]=5  [1][2]=6
                 *   [2][0]=7  [2][1]=8  [2][2]=9
                 *   [3][0]=*  [3][1]=0  [3][2]=#
                 *
                 * Color table (Table 1):
                 *   Red    -> 1, 7
                 *   Green  -> 2, 8
                 *   Blue   -> 3, 9
                 *   Yellow -> 4, *
                 *   Purple -> 5, 0
                 *   Cyan   -> 6, #
                 */

                if(keys[0][0] == 0 && last_state != 1)
                {
                    last_state = 1;
                    SW_Tx_UART_PutString("Button 1 pressed -> Red");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 1, 1);
                }
                else if(keys[0][1] == 0 && last_state != 2)
                {
                    last_state = 2;
                    SW_Tx_UART_PutString("Button 2 pressed -> Green");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 0, 1);
                }
                else if(keys[0][2] == 0 && last_state != 3)
                {
                    last_state = 3;
                    SW_Tx_UART_PutString("Button 3 pressed -> Blue");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 1, 0);
                }
                else if(keys[1][0] == 0 && last_state != 4)
                {
                    last_state = 4;
                    SW_Tx_UART_PutString("Button 4 pressed -> Yellow");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 0, 1);
                }
                else if(keys[1][1] == 0 && last_state != 5)
                {
                    last_state = 5;
                    SW_Tx_UART_PutString("Button 5 pressed -> Purple");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 1, 0);
                }
                else if(keys[1][2] == 0 && last_state != 6)
                {
                    last_state = 6;
                    SW_Tx_UART_PutString("Button 6 pressed -> Cyan");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 0, 0);
                }
                else if(keys[2][0] == 0 && last_state != 7)
                {
                    last_state = 7;
                    SW_Tx_UART_PutString("Button 7 pressed -> Red");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 1, 1);
                }
                else if(keys[2][1] == 0 && last_state != 8)
                {
                    last_state = 8;
                    SW_Tx_UART_PutString("Button 8 pressed -> Green");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 0, 1);
                }
                else if(keys[2][2] == 0 && last_state != 9)
                {
                    last_state = 9;
                    SW_Tx_UART_PutString("Button 9 pressed -> Blue");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 1, 0);
                }
                else if(keys[3][0] == 0 && last_state != 10)
                {
                    last_state = 10;
                    SW_Tx_UART_PutString("Button * pressed -> Yellow");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 0, 1);
                }
                else if(keys[3][1] == 0 && last_state != 11)
                {
                    last_state = 11;
                    SW_Tx_UART_PutString("Button 0 pressed -> Purple");
                    SW_Tx_UART_PutCRLF();
                    setLED(0, 1, 0);
                }
                else if(keys[3][2] == 0 && last_state != 12)
                {
                    last_state = 12;
                    SW_Tx_UART_PutString("Button # pressed -> Cyan");
                    SW_Tx_UART_PutCRLF();
                    setLED(1, 0, 0);
                }

                printMatrix();
            }
        }
    }
}