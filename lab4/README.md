# Lab4 
###### tags: `FreeRTOS`

**Lab要求**  
* Step1:  
	* Use Uart to show Free linked list.  
	* Create a task to call vPrintFreeList.  
	* Implement your vPrintFreeList function in heap_2.c.  
* Step2:  
	* Modifying prvInsertBlockIntoFreeList code in heap_2.c to implement memory merge.  
		* Create six tasks  
		* RedLEDTask  
		* GreenLEDTask  
		* Task1  
		* Task2  
		* Task3  
		* PrintTask  
Task1、Task2、Task3 will delete themselves.  
PrintTask will call vPrintFreeList function to show free list information.  

**output:**  
1: ![](https://hackmd.io/_uploads/ry2aZSSE2.png)  

2: ![](https://hackmd.io/_uploads/Hk_2ZBrV3.png)  

**介紹:**  
![](https://hackmd.io/_uploads/H1lQzBBVh.png)    

FreeList 是一條起點為 xStart，終點為 xEnd 的一條 linked-list
也就是說，vPrintFreeList() 只需要走訪這條 FreeList 並取出我們需要的數值，再透過 UART 輸出即可    

heap2.c  
* Heap_2 記憶體分配使用的是best fit algorithm  
* Heap_2 支持記憶體回收，但是不會把相鄰的回收後的記憶體做合併，所以有可能造成記憶體破碎化。  
* 適合用在雖然需要釋放記憶體但是每個task需要的TCB、Stack是相同的。這樣釋放後的記憶體空間就能直接被其他的task使用，可以避免記憶體破碎化。  

main.c
```c=
int main(void)
{ 
	...
  xTaskCreate(RedLEDTask, "RedLEDTask", 100, NULL, 0, NULL);
  xTaskCreate(Task1, "Task1", 50, NULL, 0, NULL);
  xTaskCreate(Task2, "Task2", 30, NULL, 0, NULL);
  xTaskCreate(GreenLEDTask, "GreenLEDTask", 130, NULL, 0, NULL);
  xTaskCreate(Task3, "Task3", 40, NULL, 0, NULL);
  xTaskCreate(PrintTask, "PrintTask", 130, NULL, 0, NULL);
	...
}

void RedLEDTask(void const *argument) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
        vTaskDelay(500);
    }
}

void GreenLEDTask(void const *argument) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
        vTaskDelay(1000);
    }
}

void Task1(void const *argument) {
	while (1) {
        vTaskDelete(NULL);
    }
}
void Task2(void const *argument) {
    while (1) {
        vTaskDelete(NULL);
    }
}

void Task3(void const *argument) {
    while (1) {
        vTaskDelete(NULL);
    }
}
void PrintTask(void const *argument) {
    while (1) {
        vPrintFreeList();
        vTaskDelay(3000);
	}
}

```
>  main.c 修改內容: 新增 6 個 Tasks 以觀察記憶體使用情況
> * red_LED_task, green_LED_task: 持續閃爍 LED，會持續占用同一塊記憶體
> * task1, task2, task3: 刪除自己的 task，用來釋放記憶體
> * print_task: 定期呼叫 vPrintFreeList 以觀察記憶體使用狀況

heap2.c  
**no merge -> output 1**
```c=
/*
 * Insert a block into the list of free blocks - which is ordered by size of
 * the block.  Small blocks at the start of the list and large blocks at the end
 * of the list.
 */
#define prvInsertBlockIntoFreeList( pxBlockToInsert )                                                                               \
{                                                                                                                               \
    BlockLink_t * pxIterator;                                                                                                   \
    size_t xBlockSize;                                                                                                          \
                                                                                                                                    \
    xBlockSize = pxBlockToInsert->xBlockSize;                                                                                   \
                                                                                                                                    \
    /* Iterate through the list until a block is found that has a larger size */                                                \
    /* than the block we are inserting. */                                                                                      \
    for( pxIterator = &xStart; pxIterator->pxNextFreeBlock->xBlockSize < xBlockSize; pxIterator = pxIterator->pxNextFreeBlock ) \
    {                                                                                                                           \
        /* There is nothing to do here - just iterate to the correct position. */                                               \
    }                                                                                                                           \
                                                                                                                                    \
    /* Update the list to include the block being inserted in the correct */                                                    \
    /* position. */                                                                                                             \
    pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;                                                             \
    pxIterator->pxNextFreeBlock = pxBlockToInsert;                                                                              \
}
```
**merge -> output2**
```c=
#define prvInsertBlockIntoFreeList( pxBlockToInsert )								\
{																					\
	BlockLink_t * pxIterator;                                                                                                          \
	    size_t xBlockSize;                                                                                                                 \
	                                                                                                                                       \
	    xBlockSize = pxBlockToInsert->xBlockSize;                                                                                          \
	                                                                                                                                       \
	    /* merge begin */                                                                                                                  \
	    _Bool merge = 0;                                                                                                                   \
	    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock != &xEnd; pxIterator = pxIterator->pxNextFreeBlock)                         \
	    {                                                                                                                                  \
	        BlockLink_t* tmp = pxIterator->pxNextFreeBlock;                                                                                \
	        StackType_t endaddress = (StackType_t) pxBlockToInsert + (StackType_t) xBlockSize;                                             \
	        StackType_t tmpstartaddress = (StackType_t) tmp;                                                                               \
	        if (endaddress == tmpstartaddress)                                                                                             \
	        {                                                                                                                              \
	            pxBlockToInsert->pxNextFreeBlock = tmp->pxNextFreeBlock;                                                                   \
	            pxBlockToInsert->xBlockSize += tmp->xBlockSize;                                                                            \
	            pxIterator->pxNextFreeBlock = pxBlockToInsert;                                                                             \
	                                                                                                                                       \
	            merge = 1;                                                                                                                 \
	            break;                                                                                                                     \
	        }                                                                                                                              \
	                                                                                                                                       \
	        StackType_t startaddress = (StackType_t) pxBlockToInsert;                                                                      \
	        StackType_t tmpendaddress = (StackType_t) tmp + (StackType_t) (tmp->xBlockSize);                                               \
	        if (startaddress == tmpendaddress)                                                                                             \
	        {                                                                                                                              \
	            tmp->xBlockSize += pxBlockToInsert->xBlockSize;                                                                            \
	                                                                                                                                       \
	            merge = 1;                                                                                                                 \
	            break;                                                                                                                     \
	        }                                                                                                                              \
	    }                                                                                                                                  \
	                                                                                                                                       \
	    if (!merge) {                                                                                                                      \
	        /* Iterate through the list until a block is found that has a larger size */                                                   \
	        /* than the block we are inserting. */                                                                                         \
	        for( pxIterator = &xStart; pxIterator->pxNextFreeBlock->xBlockSize < xBlockSize; pxIterator = pxIterator->pxNextFreeBlock )    \
	        {                                                                                                                              \
	            /* There is nothing to do here - just iterate to the correct position. */                                                  \
	        }                                                                                                                              \
	                                                                                                                                       \
	        /* Update the list to include the block being inserted in the correct */                                                       \
	        /* position. */                                                                                                                \
	        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;                                                                \
	        pxIterator->pxNextFreeBlock = pxBlockToInsert;                                                                                 \
	    }\
}
```
新增vPrintFreeList 函式，印出地址與block資訊  
  
```c=

void vPrintFreeList(void) {
	char *TITLE = "StartAddress\t|heapSTRUCT_SIZE\t|xBlockSize\t|EndAddress\r\n";
	char buf[64], start_addr[16], end_addr[16];
	memset(buf, '\0', sizeof(buf));
	memset(start_addr, '\0', sizeof(start_addr));
	memset(end_addr, '\0', sizeof(end_addr));

	HAL_UART_Transmit(&huart2, (uint8_t *)TITLE, strlen(TITLE), 0xffff);
	BlockLink_t *pBlock = xStart.pxNextFreeBlock;
	while (pBlock != &xEnd) {
		memset(buf, '\0', sizeof(buf));
		memset(start_addr, '\0', sizeof(start_addr));
		memset(end_addr, '\0', sizeof(end_addr));
		Uint32ConvertHex((uint32_t)pBlock, start_addr);
		Uint32ConvertHex((uint32_t)pBlock + (uint32_t)(pBlock->xBlockSize), end_addr);
		sprintf(buf, "%s\t%d\t%d\t%s\r\n", start_addr, heapSTRUCT_SIZE,
				                           pBlock->xBlockSize, end_addr);
		HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), 0xffff);
		pBlock = pBlock->pxNextFreeBlock;
	}
}
```
task.h 修改內容: 宣告 huart2 變數以使用 UART 輸出文字到 Console  
```c=
#include "stm32f4xx_hal.h"

extern UART_HandleTpyeDef huart2;
```