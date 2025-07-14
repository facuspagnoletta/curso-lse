#include "board.h"
#include "task.h"
#include "FreeRTOS.h"
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

    xTaskCreate(task_init,         
        "Iniciar periféricos",
        tskINIT_STACK,
        NULL,
        tskINIT_PRIORITY,
        NULL
    );


}