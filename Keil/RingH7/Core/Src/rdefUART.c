


#include "rdefUART.h"




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: transfer complete */
  UartReady = SET;

  
}





/* Definition variant of using UART without interrupt

uint8_t comm_rec[2]={0};  // recieve buffer
HAL_StatusTypeDef io_result;
int SigTime=0xFFF;

main()
{
 ...

 // wait for data with timeout in 0xFFF
 io_result = HAL_UART_Receive(&huart3, comm_rec, sizeof(comm_rec), 0xFFF);
		
 if (io_result != HAL_OK)
		{
			if(io_result == HAL_TIMEOUT)
			{
	     while(SigTime)		
		 	 {
			  HAL_GPIO_TogglePin(GPIOB, LD2B_Pin);
        HAL_Delay(100);
				--SigTime;
		   }				
			}else // HAL_ERROR || HAL_BUSY
			{
			 Error_Handler();
		  }		
		}
		else
		{
			SigTime=0xF;
	     while(SigTime)		
		 	 {
			  HAL_GPIO_TogglePin(GPIOB, LD3R_Pin);
        HAL_Delay(100);
				--SigTime;
		   }	
		 HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_RESET);	 
		}
...
}

*/