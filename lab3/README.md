# 嵌入式Lab3    

## Environment  
環境: wnidows 10, STM32CubeIDE 1.8.0  
開發板: stm32f407g  
事前設定: Motion sensor setup  
設定interrupt: EXIT line0 interupt ->enable:5    

## Requirement  
* Sensor interrupt  
* Use semephore  
* At the beginning, the green LED blinking, then shake the board, the red LED triggered（switch state） by ISR and the orange LED blinking five times in handler task.  
* When orange LED blinking, you should not trigger the sensor interrupt if you shake the board.    

## 示意圖  
![](https://i.imgur.com/WYYOAt0.png)
  
## Code  
`
main.c
`
```c=
// Private includes
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Private macro
#define LIS3DSH_WHO_AM_I_ADDR                0x0F
#define LIS3DSH_STAT_ADDR                    0x18
#define LIS3DSH_CTRL_REG4_ADDR               0x20
#define LIS3DSH_CTRL_REG1_ADDR               0x21
#define LIS3DSH_CTRL_REG2_ADDR               0x22
#define LIS3DSH_CTRL_REG3_ADDR               0x23
#define LIS3DSH_CTRL_REG5_ADDR               0x24
#define LIS3DSH_CTRL_REG6_ADDR               0x25

#define LIS3DSH_STATUS_ADDR                  0x27

#define LIS3DSH_OUT_X_L_ADDR                 0x28
#define LIS3DSH_OUT_X_H_ADDR                 0x29
#define LIS3DSH_OUT_Y_L_ADDR                 0x2A
#define LIS3DSH_OUT_Y_H_ADDR                 0x2B
#define LIS3DSH_OUT_Z_L_ADDR                 0x2C
#define LIS3DSH_OUT_Z_H_ADDR                 0x2D

#define LIS3DSH_ST1_1_ADDR                   0x40
#define LIS3DSH_ST1_2_ADDR                   0x41
#define LIS3DSH_THRS1_1_ADDR                 0x57
#define LIS3DSH_MASK1_B_ADDR                 0x59
#define LIS3DSH_MASK1_A_ADDR                 0x5A
#define LIS3DSH_SETT1_ADDR                   0x5B
#define LIS3DSH_OUTS1_ADDR                   0x5F
#define LONG_TIME 0xffff
```

```c=
SemaphoreHandle_t xSemaphore=NULL;
void MEMS_Write(uint8_t address,uint8_t data){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1,&address,1,10);
	HAL_SPI_Transmit(&hspi1,&data,1,10);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3,GPIO_PIN_SET);
}
void MEMS_Read(uint8_t address,uint8_t *data){
    address |= 0x80;
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1,&address,1,10);
	HAL_SPI_Receive(&hspi1,data,1,10);
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_SET);
}

void Green_LED_Task(void *pvParameters)
{
		for(;;){
			HAL_GPIO_WritePin(led_green_GPIO_Port,GPIO_PIN_12,GPIO_PIN_SET);
			vTaskDelay(1000);

			HAL_GPIO_WritePin(led_green_GPIO_Port,GPIO_PIN_12,GPIO_PIN_RESET);
			vTaskDelay(1000);
		}
}
void vHandlerTask( void *pvParameters )
{

	for(;;)
	{
        /* Take the semaphore */

            if(xSemaphoreTake(xSemaphore,LONG_TIME)==pdTRUE){
    /* orange led toggle */
            uint8_t data = 0;
            for(int i=0;i<10;i++){
                uint32_t From_begin_time = HAL_GetTick();
                HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
                //不使用vTaskDelay方式閃爍
                while(HAL_GetTick() - From_begin_time < 250/portTICK_RATE_MS)
                {
                    ;
                }
            }
            MEMS_Read(0x5F,&data);

    }

    }
}
//當開發版搖動時會觸發這個函式
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* red led toggle */


	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
	/* Give the semaphore to unblock the handler task */

}

int main(void)
{
    ...
  xSemaphore = xSemaphoreCreateBinary();

  // task create //

  xTaskCreate(Green_LED_Task,"Green_LED_Task", 1000, NULL,1, NULL);
  xTaskCreate(vHandlerTask,"vHandlerTask", 1000, NULL,3, NULL);
  vTaskStartScheduler();
    ...
}
```