#include "board.h"

#define LED_Blue  GPIO,1,1
#define LED_D1   GPIO,0,29



int main (void) {

    //Inicializo el clock del microcontrolador
    BOARD_BootClockFRO24M();

    //Inicializo puerto 1
    GPIO_PortInit (GPIO,1);
    //Inicializo el puerto 0 
    GPIO_PortInit (GPIO,0);
    
    //Inicializo el pin 1 del puerto 1 como salida
    gpio_pin_config_t out_config = {.pinDirection = kGPIO_DigitalOutput, .outputLogic=1 };


    GPIO_PinInit(LED_Blue,&out_config);
    GPIO_PinInit(LED_D1,&out_config);

     // Configuro SysTick para 1 ms
    SysTick_Config(SystemCoreClock / 1000);

    while (1) {
    }
    return 0;
}

void SysTick_Handler(void) {
	// Variable para contar interrupciones
	static uint16_t i = 0;

	// Incremento contador
	i++;
	// Verifico si el SysTick se disparo 500 veces (medio segundo)
	if(i == 500) {
		// Reinicio el contador
		i = 0;
        GPIO_PinWrite(LED_Blue,GPIO_PinRead(LED_Blue)); //Enciendo el led
        
        
    }
    if(i==1500) {
        i = 0;
        GPIO_PinWrite(LED_D1,GPIO_PinRead(LED_D1)); //Apago el led
    }
}