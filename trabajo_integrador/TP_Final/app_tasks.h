#ifndef _APP_TASKS_H_
#define _APP_TASKS_H_

#include "board.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "labels.h"   
#include "wrappers.h" 
#include "isr.h"      

extern TaskHandle_t handle_tricolor;
extern TaskHandle_t handle_display; 

#define tskINIT_PRIORITY             (tskIDLE_PRIORITY + 3UL)
#define tskADC_PRIORITY              (tskIDLE_PRIORITY + 1UL)
#define tskDISPLAY_CHANGE_PRIORITY   (tskIDLE_PRIORITY + 2UL)
#define tskCONTROL_PRIORITY          (tskIDLE_PRIORITY + 1UL)
#define tskDISPLAY_PRIORITY          (tskIDLE_PRIORITY + 1UL)
#define tskRV22_PRIORITY             (tskIDLE_PRIORITY + 2UL) 
#define tskBH1750_PRIORITY           (tskIDLE_PRIORITY + 1UL)
#define tskBUZZER_PRIORITY           (tskIDLE_PRIORITY + 2UL)
#define tskSETPOINT_PRIORITY         (tskIDLE_PRIORITY + 2UL)
#define tskTRICOLOR_PRIORITY         (tskIDLE_PRIORITY + 1UL)
#define tskLOGGER_PRIORITY           (tskIDLE_PRIORITY + 1UL)

#define tskINIT_STACK                (3 * configMINIMAL_STACK_SIZE)
#define tskADC_STACK                 (configMINIMAL_STACK_SIZE)
#define tskDISPLAY_CHANGE_STACK      (configMINIMAL_STACK_SIZE)
#define tskCONTROL_STACK             (configMINIMAL_STACK_SIZE)
#define tskDISPLAY_STACK             (3 * configMINIMAL_STACK_SIZE / 2)
#define tskRV22_STACK                (3 * configMINIMAL_STACK_SIZE / 2)
#define tskBH1750_STACK              (configMINIMAL_STACK_SIZE)
#define tskBUZZER_STACK              (3 * configMINIMAL_STACK_SIZE)
#define tskSETPOINT_STACK            (configMINIMAL_STACK_SIZE)
#define tskTRICOLOR_STACK            (configMINIMAL_STACK_SIZE)
#define tskLOGGER_STACK              (configMINIMAL_STACK_SIZE)


void task_init(void *params);
void task_adc(void *params);
void task_display_change(void *params);
void task_control(void *params);
void task_display(void *params);
void task_bh1750(void *params);
void task_cny70(void *params);
void task_setpoint(void *params);
void task_tricolor(void *params);
void task_monitoreo(void *params);
void task_Led_Azul_rv22(void *params);


static inline bool wrapper_btn_get_with_debouncing_with_pull_up(gpio_t btn) {
    if(!wrapper_btn_get(btn)) {
        vTaskDelay(pdMS_TO_TICKS(20));
        if(!wrapper_btn_get(btn)) {
            return true;
        }
    }
    return false;
}

#endif 
