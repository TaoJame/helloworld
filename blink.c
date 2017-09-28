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

   the other two LED are assigned to show the UART status
   so the only one LED for user is connected to GPIO25
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

SemaphoreHandle_t xMutex;

bool flag_Going;
int count;
void vApplicationIdleHook()
{
}

void TestVoidPoiter(void *pvParameter)
{
    int *p_int = (int *)pvParameter;
    printf("test TestVoidPoiter: %d\r\n", *p_int);
}

void blink_fast_task(void *pvParameter)
{
    int i = 50;
    bool s = 1;
    TaskHandle_t xIdleHandle;
    while (1)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        printf("%s: blink for 50 times\r\n", (char *)pvParameter);
        while (i--)
        {
            s = !s;
            gpio_set_level(BLINK_GPIO, s);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        printf("%s: finish blink and suspend\r\n\r\n\r\n ", (char *)pvParameter);
        // attention: you must give back the semaphore before Suspend the task
        xSemaphoreGive(xMutex);
        xIdleHandle = xTaskGetCurrentTaskHandle();
        vTaskSuspend(xIdleHandle);
    }
}
void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    int *p_int = (int *)pvParameter;
    TaskHandle_t localHandler = NULL;
    int i = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        i++;
        if (i == 5)
        {
            /*create a high priority task, block for 5s*/
            if (*p_int == 1)
            {
                xTaskCreate(&blink_fast_task, "blink_fast_task1", 2048, "Created by other task1", 5, &localHandler);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            else
            {
                xTaskCreate(&blink_fast_task, "blink_fast_task2", 2048, "Created by other task2", 5, &localHandler);
                //to check out if task is periodic, it doesn't Delay when create a higher priority task
            }
        }
        else if (i == 10)
        {
            i = 0;
            vTaskDelete(localHandler);
            /*delete a high priority task*/
        }

        xSemaphoreTake(xMutex, portMAX_DELAY);
        if (i == 0)
        {
            printf("task%d delete delete delete the task\r\n\r\n", *p_int);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        TestVoidPoiter(&i);
        {

            if (*p_int == 1)
            {
                /* Blink off (output low) */
                gpio_set_level(BLINK_GPIO, 0);
                printf("blink task%d Blink off \r\n\r\n", *p_int);
                vTaskDelay(300 / portTICK_PERIOD_MS);
            }
            else if (*p_int == 2)
            {
                /* Blink on (output high) */
                gpio_set_level(BLINK_GPIO, 1);
                printf("blink task%d Blink on for fixed time, and last time it woke up is%d\r\n\r\n", *p_int, xLastWakeTime);
                vTaskDelayUntil(&xLastWakeTime, 700 / portTICK_PERIOD_MS);
            }
        }
        xSemaphoreGive(xMutex);

        /* Blink off (output low) */
        //gpio_set_level(BLINK_GPIO, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        //gpio_set_level(BLINK_GPIO, 1);
        //vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int task1 = 1; //如果放在app_main里面就有问题，因为在堆栈里面，到了新的任务里面就可能不见了
int task2 = 2; //放在静态缓冲区就非常好

void app_main()
{

    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    xMutex = xSemaphoreCreateMutex();

    xTaskCreate(&blink_task, "blink_task1", 2048, &task1, 2, NULL); //set the STACK SIZE bigger
    xTaskCreate(&blink_task, "blink_task2", 2048, &task2, 2, NULL);
    //xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
