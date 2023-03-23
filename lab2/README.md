# 嵌入式 Lab2    

## Environment  
環境: wnidows 10, STM32CubeIDE 1.8.0  
開發板: stm32f407g  
使用 uart設置好 uart4    

## Requirement  
* Create four task  
    * Red_LED_App、Green_LED_App、Delay_App、TaskMonitor_App  
* TaskMonitor_App will call Taskmonitor() periodicity  
* TaskMonitor()  
    * Traverse **ReadyTaskList, DelayedTaskList, OverflowDelayedTaskList**  
    * Print TCB information by UART
        * Task Name、Priority(Base/actual)、Stack Pointer、Topofstack Pointer、Task State    

## Result  
![](https://i.imgur.com/YlNBpY6.png)    

## Background  
TaskList 的資料結構為 Linked List    

![](https://i.imgur.com/mXgBPf4.png)    


## Code  


`main.c`  
加入include細項  
```c=
#include "stdio.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"
#include "List.h"
...
...    
int main(){
...
  TaskHandle_t xHandle=NULL;
  xTaskCreate(Red_LED_App, "Red_LED_app", 128, NULL, 1, &xHandle);
  xTaskCreate(Green_LED_App, "Green_LED_app", 128, NULL, 1, &xHandle);
  xTaskCreate(Delay_App, "Delay_app", 128, NULL, 14, &xHandle);
  xTaskCreate( TaskMonitor_App,  "TaskMonitor_App", 512,NULL,3,&xHandle);
  ...
}

void TaskMonitor_App(void *pvParameters){
	for(;;){
		Taskmonitor();
		vTaskDelay(1000);
	}
}

void Red_LED_App(void *pvParameters){
	uint32_t Redtimer = 800;
	for(;;){
		HAL_GPIO_TogglePin(led_red_GPIO_Port,GPIO_PIN_14);
		vTaskDelay(Redtimer);
		Redtimer+=1;
	}
}

void Green_LED_App(void *pvParameters){
	uint32_t Greentimer = 1000;
	for(;;){
		HAL_GPIO_TogglePin(led_green_GPIO_Port,GPIO_PIN_12);
		vTaskDelay(Greentimer);
		Greentimer+=2;
	}
}
void Delay_App(void *pvParameters){
	int delayflag=0;
	uint32_t delaytime;
	while(1){
		if(delayflag==0){
			delaytime = 1000;
			delayflag=1;
		}else{
			delaytime=0xFFFFFFFF;
		}
		vTaskDelay(delaytime);
	}
}
```  
`task.h`  
```c=
//宣告Taskmonitor 函式做為task control block的資訊
UART_HandleTypeDef huart4;
void Taskmonitor(void);
```
`task.c`    

```c=
void print_pcb(TCB_t  *t, char *state) {
	char Monitor_data[250]; 
	memset(Monitor_data,'\0',sizeof(Monitor_data));
	sprintf(Monitor_data, "%-19s %3lu/%-19lu 0x%-9lx 0x%-14lx %s\n\r",t->pcTaskName, t->uxPriority, t->uxBasePriority, t->pxStack, t->pxTopOfStack, state);

	HAL_UART_Transmit(&huart4, (uint8_t *)Monitor_data, strlen(Monitor_data), HAL_MAX_DELAY);
}

void Taskmonitor(void)
{

	vTaskSuspendAll();
	char *Monitor_data = "|Name  |Priority(Base/actual) |pxStack  |pxTopOfStack  |State  |\n\r";

	HAL_UART_Transmit(&huart4,(uint8_t*)Monitor_data,strlen(Monitor_data),HAL_MAX_DELAY);
	ListItem_t *node;

	/*pxreadytaskslists*/
	for(int i=0;i<configMAX_PRIORITIES;i++){
		node = listGET_HEAD_ENTRY(pxReadyTasksLists+i);
		for(uint32_t j=0;j<listCURRENT_LIST_LENGTH(pxReadyTasksLists + i); j++)
		{
			print_pcb(listGET_LIST_ITEM_OWNER(node),"Ready");
			node = listGET_NEXT(node);
		}
	}

	/*pxDelayedTask*/
	node = listGET_HEAD_ENTRY(pxDelayedTaskList);
	for(uint32_t j=0;j<listCURRENT_LIST_LENGTH(pxDelayedTaskList);j++){
		print_pcb(listGET_LIST_ITEM_OWNER(node),"Blocked");
		node = listGET_NEXT(node);
	}

	/*pxOverflowdelayedtasklist*/
	node =  listGET_HEAD_ENTRY(pxOverflowDelayedTaskList);;
	for(uint32_t j=0;j<listCURRENT_LIST_LENGTH(pxOverflowDelayedTaskList);j++){
		print_pcb(listGET_LIST_ITEM_OWNER(node),"Overflow");
		node = listGET_NEXT(node);
	}
	xTaskResumeAll();
}
```
