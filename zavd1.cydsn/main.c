#include "project.h"
int main(void)
{
CyGlobalIntEnable; /* Enable global interrupts. */
for(;;)
{
if(Button_Read() == 1)
{
LED_Write(0);
}
else
{
LED_Write(1);
}
}
}