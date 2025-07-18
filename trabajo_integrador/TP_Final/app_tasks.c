#include "app_tasks.h" 
#include "fsl_debug_console.h" 

// Colas y Semáforos 
QueueHandle_t queue_adc;
QueueHandle_t queue_display;
QueueHandle_t queue_display_variable;
QueueHandle_t queue_lux;
QueueHandle_t queue_lux_data;
QueueHandle_t queue_setpoint;

xSemaphoreHandle semphr_buzz;
xSemaphoreHandle semphr_usr;
xSemaphoreHandle semphr_mutex;

TaskHandle_t handle_tricolor;

// Tarea 1: Iniciar de periféricos y recursos
void task_init(void *params) {
    semphr_usr = xSemaphoreCreateBinary();
    semphr_mutex = xSemaphoreCreateMutex();
    
    queue_display = xQueueCreate(1, sizeof(uint16_t));
    queue_adc = xQueueCreate(1, sizeof(adc_data_t));
    queue_display_variable = xQueueCreate(1, sizeof(display_variable_t));
    queue_lux = xQueueCreate(1, sizeof(float));
    queue_lux_data = xQueueCreate(1, sizeof(uint16_t));
    queue_setpoint = xQueueCreate(1, sizeof(float));

    // Incialización de GPIO
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
    // wrapper_gpio_enable_irq((gpio_t){CNY70}, kPINT_PinIntEnableBothEdges, cny70_callback);
    wrapper_gpio_enable_irq((gpio_t){USR_BTN}, kPINT_PinIntEnableFallEdge, usr_callback);
    // Inicializo el PWM
    wrapper_pwm_init();
    // Inicializo I2C
    wrapper_i2c_init();
    // Inicializo el BH1750
    wrapper_bh1750_
    
\\


\
init();

    vTaskDelete(NULL);
}

// Tarea 2: ADC
void task_adc(void *params) {
    while (1) {
        ADC_DoSoftwareTriggerConvSeqA(ADC0);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// Tarea 3: Control del botón de usuario
void task_display_change(void *params) {
    display_variable_t variable = kDISPLAY_LUX;
    while (1) {
        xQueueOverwrite(queue_display_variable, &variable);
        xSemaphoreTake(semphr_usr, portMAX_DELAY);
        variable = (variable == kDISPLAY_LUX) ? kDISPLAY_SETPOINT : kDISPLAY_LUX;
    }
}

// Tarea 4: Controla qué dato se envía al display
void task_control(void *params) {
    display_variable_t variable_actual = kDISPLAY_LUX;
    float lux_percent = 0;
    float current_setpoint = 0;
    uint16_t val = 0;

    while (1) {
        xQueuePeek(queue_display_variable, &variable_actual, portMAX_DELAY);

        if (variable_actual == kDISPLAY_LUX) {
            xQueuePeek(queue_lux, &lux_percent, portMAX_DELAY);
            val = (uint16_t)lux_percent;
        } else { 
            xQueuePeek(queue_setpoint, &current_setpoint, portMAX_DELAY);
            val = (uint16_t)current_setpoint;
        }

        if (val > 99) {
            val = 99;
        }
        
        xQueueOverwrite(queue_display, &val);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarea 5: Escribe un número en el display 

void task_display(void *params) {
    uint16_t data_to_write;
    display_variable_t variable_actual;
    while (1) {
        if (!xQueuePeek(queue_display, &data_to_write, pdMS_TO_TICKS(100))) {
            continue;
        }
        xQueuePeek(queue_display_variable, &variable_actual, 0);

        wrapper_display_off();
        wrapper_display_write((uint8_t)(data_to_write / 10));
        wrapper_display_on((gpio_t){COM_1});
        vTaskDelay(pdMS_TO_TICKS(10));

        wrapper_display_off();
        wrapper_display_write((uint8_t)(data_to_write % 10));
        wrapper_display_on((gpio_t){COM_2});
        
        // Muestra el punto decimal si corresponde
        if (variable_actual == kDISPLAY_SETPOINT) {
        
            wrapper_display_segment_on((gpio_t){SEG_DP});
        } else {
            wrapper_display_segment_off((gpio_t){SEG_DP});
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Tarea 6: Control de LED Azul con RV22 

void task_Led_Azul_rv22(void *params) {
    adc_data_t adc_data;
    int16_t manual_duty = 0;

    while (1) {
        xQueuePeek(queue_adc, &adc_data, portMAX_DELAY);
        
        manual_duty = (adc_data.temp_raw * 100) / 4095;

        wrapper_pwm_update_led_azul(manual_duty);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarea 7: Sensor BH1750

void task_bh1750(void *params) {
    uint16_t lux_raw_value = 0;
    float lux_por = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(200));
        lux_raw_value = wrapper_bh1750_read();
        if (lux_raw_value > 30000) lux_raw_value = 30000;
        lux_por = (float)lux_raw_value * 100.0f / 30000.0f;
        xQueueOverwrite(queue_lux, &lux_por);
        xQueueOverwrite(queue_lux_data, &lux_raw_value);
    }
}

// Tarea 8: Buzzer

// void task_buzzer(void *params) {
//     while (1) 
//     {
//         xSemaphoreTake(semphr_buzz, portMAX_DELAY);
//         // Enciende el buzzer
//         wrapper_output_toggle((gpio_t){BUZZER});
//     }
        
// }

// Tarea 9: Setpoint

void task_setpoint(void *params) {
    float current_setpoint = 25.0f;
    while (1) {
        if (wrapper_btn_get_with_debouncing_with_pull_up((gpio_t){S1_BTN})) {
            if (current_setpoint < 75.0f) current_setpoint += 1.0f;
            while (wrapper_btn_get((gpio_t){S1_BTN})) vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (wrapper_btn_get_with_debouncing_with_pull_up((gpio_t){S2_BTN})) {
            if (current_setpoint > 25.0f) current_setpoint -= 1.0f;
            while (wrapper_btn_get((gpio_t){S2_BTN})) vTaskDelay(pdMS_TO_TICKS(10));
        }
        xQueueOverwrite(queue_setpoint, &current_setpoint);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarea 10: LEDs tricolor 

void task_tricolor(void *params) {
    float lux_percent = 0;
    float current_setpoint = 0;
    int16_t error = 0;
    const float DEADZONE = 1.0f;

    while (1) {
        xQueuePeek(queue_lux, &lux_percent, portMAX_DELAY);
        xQueuePeek(queue_setpoint, &current_setpoint, portMAX_DELAY);
        
        error = lux_percent - current_setpoint;

        if (error > DEADZONE) {
            wrapper_pwm_update_rled(error);
            wrapper_pwm_update_bled(0);
        } else if (error < -DEADZONE) {
            wrapper_pwm_update_bled(-error);
            wrapper_pwm_update_rled(0);
        } else {
            wrapper_pwm_update_rled(0);
            wrapper_pwm_update_bled(0);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarea 11: Monitor de consola

void task_monitoreo(void *params) {
//     float lux_percent = 0;
//     float current_setpoint = 0;
//     uint32_t elapsed_time_ms = 0;
//     while (1) {
//         xQueuePeek(queue_lux, &lux_percent, 0);
//         xQueuePeek(queue_setpoint, &current_setpoint, 0);
        
//         elapsed_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
//         PRINTF("Tiempo: %lu ms | Lux: %.1f%% | Setpoint: %.1f%%\r\n", elapsed_time_ms, lux_percent, current_setpoint);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}

void task_cny70(void *params) {


	while(1){
		
		if(!GPIO_PinRead(CNY70)) {
			vTaskDelay(pdMS_TO_TICKS(50));
			if(GPIO_PinRead(CNY70)) {
				// Flanco ascendente
				wrapper_output_toggle((gpio_t){BUZZER});
			}
		} else {
			vTaskDelay(pdMS_TO_TICKS(50));
			if(!GPIO_PinRead(CNY70)) {
				// Flanco descendente
				wrapper_output_toggle((gpio_t){BUZZER});
			}
		}

		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
