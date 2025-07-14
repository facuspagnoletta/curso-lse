
#include "app_tasks.h"

// Cola para datos del ADC
xQueueHandle queue_adc;
// Cola para datos del display
xQueueHandle queue_display;
// Cola para selecion de valor para el display
xQueueHandle queue_display_variable;
// Cola para datos de luminosidad
xQueueHandle queue_lux;
// cola para datos de lux, pero en crudo
xQueueHandle queue_lux_data;

// Semáforo para interrupción del infrarojo
xSemaphoreHandle semphr_buzz;
// Semáforo para interrupción del user button
xSemaphoreHandle semphr_usr;

// es un Handler para la tarea de display 
TaskHandle_t handle_display;

/**
 * Iniciamos los periféricos
 */
void task_init(void *params) {
	// Inicio semáforos
	semphr_buzz = xSemaphoreCreateBinary();
	semphr_usr = xSemaphoreCreateBinary();
	semphr_touch = xSemaphoreCreateBinary();
	semphr_counter = xSemaphoreCreateCounting(99, 30);
	semphr_mutex = xSemaphoreCreateMutex();

	// Inicio colas
	queue_adc = xQueueCreate(1, sizeof(adc_data_t));
	queue_display_variable = xQueueCreate(1, sizeof(display_variable_t));
	queue_lux = xQueueCreate(1, sizeof(uint16_t));
	queue_display = xQueueCreate(1, sizeof(uint16_t));
	
	// Inicializacion de GPIO
	wrapper_gpio_init(0);
	wrapper_gpio_init(1);
	// Inicialización del LED
	wrapper_output_init((gpio_t){LED}, true);
	// Inicialización del buzzer
	wrapper_output_init((gpio_t){BUZZER}, false);
	// Inicialización del enable del CNY70
	wrapper_output_init((gpio_t){CNY70_EN}, true);
	// Configuro el ADC
	wrapper_adc_init();
	// Configuro el display
	wrapper_display_init();
	// Configuro botones
	wrapper_btn_init();
	// Configuro interrupción por flancos para el infrarojo y para el botón del user
	wrapper_gpio_enable_irq((gpio_t){CNY70}, kPINT_PinIntEnableBothEdges, cny70_callback);
	wrapper_gpio_enable_irq((gpio_t){USR_BTN}, kPINT_PinIntEnableFallEdge, usr_callback);
	// Inicializo el PWM
	wrapper_pwm_init();
	// Inicializo I2C y Bh1750
	wrapper_i2c_init();
	wrapper_bh1750_init();
	// Inicializo el pulsador capacitivo
	wrapper_touch_init();

	// Elimino tarea para liberar recursos
	vTaskDelete(NULL);
}

/**
 * Tarea del ADC (1)
 */
void task_adc(void *params) {

	while(1) {
		// Inicio una conversion
		ADC_DoSoftwareTriggerConvSeqA(ADC0);
		// Bloqueo la tarea por 250 ms
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}

/**
 * Tarea Control del display (2)
 */
void task_display_change(void *params) {
	// Dato para pasar
	display_variable_t variable = kDISPLAY_TEMP;

	while(1) {
		// Escribe el dato en la cola
		xQueueOverwrite(queue_display_variable, &variable);
		// Intenta tomar el semáforo
		xSemaphoreTake(semphr_usr, portMAX_DELAY);
		// Si se presionó, cambio la variable
		variable = (variable == kDISPLAY_TEMP)? kDISPLAY_REF : kDISPLAY_TEMP;
	}
}

/**
 *  Tarea Escribir en el display (3)
 */
void task_control(void *params) {
	// Variable a mostrar
	display_variable_t variable = kDISPLAY_TEMP;
	// Valores de ADC
	adc_data_t data = {0};
	// Valor a mostrar
	uint16_t val = 0;

	while(1) {
		// Veo que variable hay que mostrar
		xQueuePeek(queue_display_variable, &variable, portMAX_DELAY);
		// Leo los datos del ADC
		xQueuePeek(queue_adc, &data, portMAX_DELAY);
		// Veo cual tengo que mostrar
		val = (variable == kDISPLAY_TEMP)? data.temp_raw : data.ref_raw;
		val = 30 * val / 4095;
		// Escribo en la cola del display si puedo tomar el mutex
		xSemaphoreTake(semphr_mutex, portMAX_DELAY);
		xQueueOverwrite(queue_display, &val);
		xSemaphoreGive(semphr_mutex);

		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

/**
 *  Tarea Número en el display (4)
 */
void task_display(void *params) {
	// Variable con el dato para escribir
	uint8_t data;

	while(1) {
		// Mira el dato que haya en la cola
		if(!xQueuePeek(queue_display, &data, pdMS_TO_TICKS(100))) { continue; }
		// Muestro el número
		wrapper_display_off();
		wrapper_display_write((uint8_t)(data / 10));
		wrapper_display_on((gpio_t){COM_1});
		vTaskDelay(pdMS_TO_TICKS(10));
		wrapper_display_off();
		wrapper_display_write((uint8_t)(data % 10));
		wrapper_display_on((gpio_t){COM_2});
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


/**
 *  Tarea LED Azul y RV22 (5)
 */
void task_blinky(void *params) {
	// Variable para guardar el tiempo en ms de bloqueo
	uint16_t blocking_time;

	while(1) {
		// Lee el último valor de luminosidad
		xQueuePeek(queue_lux, &blocking_time, portMAX_DELAY);
		// Máximo es aprox 30000 entonces 3000 ms como máximo
		blocking_time /= 10;
		// Conmuto salida
		wrapper_output_toggle((gpio_t){LED});
		// Bloqueo el tiempo que se indique de la cola
		vTaskDelay(pdMS_TO_TICKS(blocking_time));
	}
}

/**
 *  Tarea para Buzzer (6)
 */
void task_buzzer(void *params) {

	while(1) {
		// Intenta tomar el semáforo
		xSemaphoreTake(semphr_buzz, portMAX_DELAY);
		// Conmuto el buzzer
		wrapper_output_toggle((gpio_t){BUZZER});
	}
}

/**
 *  Tarea para BH1750 (7)
 */
void task_BH1750(void *params) {
    //Parámetros de luminosidad
    uint16_t lux = 0 ;
    float lux_p = 0;

    while (1){
        // Bloqueo
        vTaskDelay(pdMS_TO_TICKS(200));

        // Leo el valor de lux
        lux = wrapper_bh1750_read();
        if (lux > 30000)
            lux = 30000;

        lux_p = (lux / 30000.0f) * 100.0f;

        // Muestrar en la consola
        xQueueOverwrite(queue_lux, &lux_pct);
        xQueueOverwrite(queue_lux_raw, &lux);
    }
}

/**
 *  Tarea para Buzzer (8)
 */
void task_buzzer(void *params) {

	while(1) {
		// Intenta tomar el semáforo
		xSemaphoreTake(semphr_buzz, portMAX_DELAY);
		// Conmuto el buzzer
		wrapper_output_toggle((gpio_t){BUZZER});
	}
}

/**
 *  Tarea Setpoint (9)
 */

/**
 *  Tarea Tricolor (10)
 */
/**
 *  Tarea Monitoreo (11)
 */