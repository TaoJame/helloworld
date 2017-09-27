/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

SemaphoreHandle_t xMutex;


void TestVoidPoiter(void* pvParameter)
{
    int *p_int = (int *)pvParameter;
    printf("test TestVoidPoiter: %d\r\n",*p_int);    
}

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    int *p_int = (int*)pvParameter;
    int i=0;
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        xSemaphoreTake( xMutex, portMAX_DELAY );
        i++;
        TestVoidPoiter(&i);
        
        {
            printf("blink task:%d running \r\n\r\n", *p_int);
        }
        xSemaphoreGive( xMutex );

        /* Blink off (output low) */
        //gpio_set_level(BLINK_GPIO, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        //gpio_set_level(BLINK_GPIO, 1);
        //vTaskDelay(pdMS_TO_TICKS(2000));
        
    }
}


int task1=1;//如果放在app_main里面就有问题，因为在堆栈里面，到了新的任务里面就可能不见了
int task2=2;//放在静态缓冲区就非常好


void app_main()
{
    xMutex=xSemaphoreCreateMutex();

    xTaskCreate(&blink_task, "blink_task1", 2048, &task1, 2, NULL);//set the STACK SIZE bigger
    xTaskCreate(&blink_task, "blink_task2", 2048, &task2, 2, NULL);
    //xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

