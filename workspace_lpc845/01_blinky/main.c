#include "board.h"

#define LED_Blue  GPIO 1,1
#define delay(t)  for (unsigned int i = 0; i < t; i++);

int main (void) {
    //Inicializo puerto 1
    GPIO_PortInit (GPIO,1);
    
    //Inicializo el pin 1 del puerto 1 como salida
    gpio_pin_config_t out_config = {.pinDirection = kGPIO_DigitalOutput, .outputLogic=1 };
    GPIO_PinInit(GPIO,1,1,&out_config);

    while (1)
    {
        GPIO_PinWrite(LED_Blue,GPIO_PinRead(LED_Blue)); //Enciendo el led
        delay(50000)
        
    }

    return 0;
}