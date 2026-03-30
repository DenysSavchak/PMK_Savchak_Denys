#include "project.h"
void Color_Yellow(void)
{
    LED_Write(0);
    LED2_Write(0);
    LED3_Write(1);
}
void Color_White(void)
{
    LED_Write(0);
    LED2_Write(0);
    LED3_Write(0);
}
void Color_Off(void)
{
    LED_Write(1);
    LED2_Write(1);
    LED3_Write(1);
}
int main(void)
{
    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF(); 
    SW_Tx_UART_PutString("Savchak debys");
    SW_Tx_UART_PutCRLF();    
    SW_Tx_UART_PutString("laboratorna robota №2 ");
    SW_Tx_UART_PutCRLF();
    CyGlobalIntEnable;
 
    for(;;)
    {
        if(Button_Read() == 1)
        {
    SW_Tx_UART_PutCRLF(); 
    SW_Tx_UART_PutString("button off");
            Color_Yellow();
            CyDelay(50);
            Color_Off();
            CyDelay(50);
        }
        else                  
        {
    SW_Tx_UART_PutCRLF(); 
    SW_Tx_UART_PutString("button on");
            Color_White();
            CyDelay(50);
            Color_Off();
            CyDelay(50);
        }
    }
}