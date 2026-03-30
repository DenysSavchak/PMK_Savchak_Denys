#include "project.h"

int main(void)
{
    CyGlobalIntEnable;
    
    int mode = 0;

    for(;;)
    {
        if(Button_Read() == 1) 
        {
            switch(mode)
            {
                case 0:
                    LED_Write(1);
                    LED2_Write(0);
                    LED3_Write(0);
                    mode = 1;
                    break;
                    
                case 1:
                    LED_Write(0);
                    LED2_Write(1);
                    LED3_Write(0);
                    mode = 2;
                    break;
                    
                case 2:
                    LED_Write(0);
                    LED2_Write(0);
                    LED3_Write(1);
                    mode = 0;
                    break;
                    
                default:
                    mode = 0;
                    break;
            }

            while(Button_Read() == 1); 
            
            CyDelay(50); 
        }
    }
}