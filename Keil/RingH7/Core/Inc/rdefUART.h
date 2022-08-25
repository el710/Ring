#ifndef RDEF_UART_H
#define RDEF_UART_H

#include "stm32h7xx.h"                  // Device header



extern __IO ITStatus UartReady; // volatile enum { SET, RESET }





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle);


#endif // RDEF_UART_H