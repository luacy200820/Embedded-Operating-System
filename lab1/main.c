#include "stdio.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/*
Define led name and pin number in main.h

#define led_green_Pin GPIO_PIN_12
#define led_green_GPIO_Port GPIOD
#define led_orange_Pin GPIO_PIN_13
#define led_orange_GPIO_Port GPIOD
#define led_red_Pin GPIO_PIN_14
#define led_red_GPIO_Port GPIOD
#define led_blue_Pin GPIO_PIN_15
*/

void LEDTask_App(void *pvParameters);
void Switch_LED(void *pvParameters);
QueueHandle_t  qh;
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  TaskHandle_t xHandle = NULL;
//  QueueHandle_t queue1;
  /* USER CODE BEGIN 2 */

  qh = xQueueCreate(1, sizeof(unsigned int));
  xTaskCreate(LEDTask_App, "LEDTask_app", 128, NULL, 1, &xHandle);
  xTaskCreate( Switch_LED,  "Switch_LED", 128,NULL,1,&xHandle);

  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void LEDTask_App(void *pvParameters)
{
	unsigned int cnt = 0;
	for(;;){
		while(cnt ==0){
			HAL_GPIO_WritePin(led_green_GPIO_Port,GPIO_PIN_12,GPIO_PIN_SET);
			xQueueReceive(qh,&cnt,2000);
			HAL_GPIO_WritePin(led_green_GPIO_Port,GPIO_PIN_12,GPIO_PIN_RESET);

			if(cnt ==1) break;
			HAL_GPIO_WritePin(led_red_GPIO_Port,GPIO_PIN_14,GPIO_PIN_SET);
			xQueueReceive(qh,&cnt,2000);
			HAL_GPIO_WritePin(led_red_GPIO_Port,GPIO_PIN_14,GPIO_PIN_RESET);
			if(cnt == 1) break;

		}

		while(cnt ==1){
			HAL_GPIO_WritePin(led_orange_GPIO_Port,GPIO_PIN_13,GPIO_PIN_SET);
			xQueueReceive(qh,&cnt,500);
			HAL_GPIO_WritePin(led_orange_GPIO_Port,GPIO_PIN_13,GPIO_PIN_RESET);
			xQueueReceive(qh,&cnt,500);
			if(cnt == 0) break;

		}
	}
}
void Switch_LED(void *pvParameters)
{
	unsigned int task = 0;
	unsigned int count = 0;
	for(;;){
		if(HAL_GPIO_ReadPin(bnt_blue_GPIO_Port,GPIO_PIN_0)){
			HAL_Delay(100);
			while(HAL_GPIO_ReadPin(bnt_blue_GPIO_Port,GPIO_PIN_0)){
				;
			}
			++count;
			if(count &0x01)
				task = 1;
			else
				task =0;
			xQueueSend(qh,(int *)&task,1);
			taskYIELD();
		}

	}
}
