#include "board.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"

    // Estructura para crear las tareas

    // xTaskCreate(TaskCode,     
    //     "Nombre",
    //     STACK_SIZE,
    //     &ucParametertoPass,
    //     tskIDLE_Priority,
    //     &xHandle
    // );

int main(void){

        BOARD_BootClockFRO30M();
        BOARD_InitDebugConsole();

        // 1. Iniciar los periféricos
        xTaskCreate(task_init,
            "Iniciar periféricos",
            tskINIT_STACK, 
            NULL, 
            tskINIT_PRIORITY, 
            NULL);

        // 2. Medicion BH1750
        xTaskCreate(task_bh1750,
            "BH1750", 
            tskBH1750_STACK, 
            NULL, 
            tskBH1750_PRIORITY, 
            NULL);

        // 3. Manejo de setpoint con botones S1 y S2
        xTaskCreate(task_setpoint,
             "Setpoint", 
             tskSETPOINT_STACK, 
             NULL, 
             tskSETPOINT_PRIORITY, 
             NULL);

        // 4. Actualización de display 7 segmentos
        xTaskCreate(task_display_change, 
            "Cambiar Display", 
            tskDISPLAY_CHANGE_STACK,
             NULL, 
            tskDISPLAY_CHANGE_PRIORITY,
             NULL);

        // 5. Control del display 
        xTaskCreate(task_control, 
            "Display Control", 
            tskCONTROL_STACK, 
            NULL, 
            tskCONTROL_PRIORITY, 
            NULL);

        // 6. Escribir en el display
        xTaskCreate(task_display, 
             "Escribir Display", 
             tskDISPLAY_STACK, 
             NULL, 
             tskDISPLAY_PRIORITY, 
             NULL);

        // 7. Control de LED tricolor
        xTaskCreate(task_tricolor, 
             "LED Tricolor", 
             tskTRICOLOR_STACK, 
             NULL, 
             tskTRICOLOR_PRIORITY, 
             NULL);

        // 8. Ajuste de LED azul con RV22
        xTaskCreate(task_Led_Azul_rv22, 
             "Led Azul RV22", 
             tskRV22_STACK, 
             NULL, 
             tskRV22_PRIORITY,
             NULL);

        xTaskCreate(task_cny70, "CNY70_Task", tskBUZZER_STACK, NULL, tskBUZZER_PRIORITY, NULL);

        // 10. Monitoreo
        xTaskCreate(task_monitoreo, 
            "Logger", 
            tskLOGGER_STACK, 
            NULL, 
            tskLOGGER_PRIORITY, 
            NULL);

        // 11. Tarea ADC
        xTaskCreate(task_adc, 
            "Tarea ADC", 
            tskADC_STACK, 
            NULL, 
            tskADC_PRIORITY, 
            NULL);

        vTaskStartScheduler();
}