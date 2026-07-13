#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  __disable_irq();
  while (1)
  {
  }
}
