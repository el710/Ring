/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdlib.h"
#include "string.h"


#include "crc32.h"
	
#include "Ring_prot.h"

#include "rdefUART.h"

#include "Ring.h"

#ifdef USE_DS3231
   #include "ds3231_for_stm32_hal.h"
#endif


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */






/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

__IO ITStatus UartReady; // volatile enum { SET, RESET }

T_WorkState WorkMode;
T_IOState IOState;

T_RINGState RingState;

T_RingIO_Head RMessHead;  // recieve header
T_RingIO_Head KvHead;
T_RingIO_SchData cod_sched_info, kvcod_sched_info;
T_RingIO_BlockInfo RingBlockInfo, kvRingBlockInfo, kvEndBlockInfo;
T_RingIO_Status RingStatus, PCSyncTime, kvPCSyncTime;

uint8_t answer_index;

uint8_t *Schedule;  
uint8_t buf_head;
uint8_t io_buf[IO_BLOCK_SIZE];

uint16_t block_size, number_block;
uint32_t block_crc32, get_crc32;


uint8_t busy_time;
uint32_t schedule_size, schedule_crc32;
uint32_t * schedule_addr;
uint32_t * schedule_size_addr;

uint8_t today_day;

uint8_t link_time;
RTC_TimeTypeDef NowTime;
RTC_DateTypeDef NowDate;
uint16_t NowYear;
RTC_TimeTypeDef PCTime;
RTC_DateTypeDef PCDate;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  	
	HAL_StatusTypeDef io_result;
	
	uint16_t ui_flag = 0;
  uint8_t ui8_buf;
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

	UartReady = RESET;
	
	WorkMode = SETUP;
  IOState = IO_CHECK;
		
	SetIdleData(&KvHead, sizeof(KvHead));	
	RingInitHead(&RMessHead,RING_IDLE_DATA);
	InitBlockInfo(&kvRingBlockInfo,0,0,0);
	InitExchSchedCom(&kvcod_sched_info, 0, 0);
	InitRingEndSched(&kvEndBlockInfo, 0, 0, 0);
	
	Schedule = NULL;
  schedule_size = 0;
	schedule_crc32 = CRC32((unsigned char*)&schedule_size, 4);

	
	schedule_addr = (uint32_t *)SCHEDULE_DATA_ADDR;
	schedule_size_addr = (uint32_t *)SCHEDULE_SIZE_ADDR;
	
	busy_time = 0;
	
	today_day = 0xFF; // more then 31
	
	RingState.el.RTC_EMB_State = RTC_NO;
	RingState.el.RTC_DS_State = RTC_NO;
	RingState.el.SchedFile = SCHED_NO;
	RingState.el.ready = RING_SETUP;
 	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

#ifdef USE_DS3231
  DS3231_Init(&hi2c1);
#endif

  io_result = HAL_UART_Receive_IT(&huart3, (uint8_t *)&RMessHead, sizeof(RMessHead));

	
#ifdef MY_DEBUG	
  if (io_result == HAL_OK)
	{
		HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_SET); // UART OK
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_SET); // UART ???
	}

	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, LD2B_Pin|LD3R_Pin, GPIO_PIN_RESET); 
#endif
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#ifdef USE_DS3231
   if(WorkMode == AUTOWORK)
   {		
		 if(RingState.el.RTC_DS_State == RTC_OK)
		 {
		 	 NowTime.Hours = DS3231_GetHour();
		   NowTime.Minutes = DS3231_GetMinute(); 
			 NowTime.Seconds = DS3231_GetSecond();
			 NowDate.Date = DS3231_GetDate();
			 NowDate.Month = DS3231_GetMonth();
			 NowYear = DS3231_GetYear();
			 NowDate.Year = NowYear % 100;
			 NowDate.WeekDay = DS3231_GetDayOfWeek();
	   } 
	 }
		
#endif		
		if(WorkMode != AUTOWORK)
		{
			HAL_RTC_GetDate(&hrtc, &NowDate, RTC_FORMAT_BIN);
			HAL_RTC_GetTime(&hrtc, &NowTime, RTC_FORMAT_BIN);	
		}
		
		if(WorkMode == SETUP)
		{
			// define RTC	status
			if(RingState.el.RTC_EMB_State == RTC_NO) // start
			{
				if(NowDate.Year == 0) RingState.el.RTC_EMB_State = RTC_RESET;
								         else	RingState.el.RTC_EMB_State = RTC_OK;
			}

#ifdef USE_DS3231				
				if(RingState.el.RTC_DS_State == RTC_NO)
				{
					io_result = HAL_I2C_Master_Transmit(&hi2c1, DS3231_I2C_ADDR << 1, 0, 1, 1000);
					io_result = HAL_I2C_Master_Receive(&hi2c1, DS3231_I2C_ADDR << 1, &ui8_buf, 1, 1000);
					
					if(io_result == HAL_OK)
					{
						if(DS3231_GetYear() == 2000) RingState.el.RTC_DS_State = RTC_RESET;
						                				else RingState.el.RTC_DS_State = RTC_OK;
				  }
				}
#endif				
		
			
#ifdef USE_DS3231			
			// sync with RTC DS3231
			if(  RingState.el.RTC_DS_State == RTC_OK
				 &&RingState.el.RTC_EMB_State == RTC_RESET	)
			{
				NowTime.Seconds = DS3231_GetSecond();
				ui8_buf = DS3231_GetSecond();
				while(ui8_buf == NowTime.Seconds)
				{
				  NowTime.Seconds = DS3231_GetSecond();
				}
				
				NowTime.Hours = DS3231_GetHour();
				NowTime.Minutes = DS3231_GetMinute();
				HAL_RTC_SetTime(&hrtc, &NowTime, RTC_FORMAT_BIN);
					
				NowDate.Date = DS3231_GetDate();
				NowDate.Month = DS3231_GetMonth();
				NowYear = DS3231_GetYear();
				NowDate.Year = NowYear % 100;
				NowDate.WeekDay = DS3231_GetDayOfWeek();
				HAL_RTC_SetDate(&hrtc, &NowDate, RTC_FORMAT_BIN);				
				RingState.el.RTC_EMB_State = RTC_OK;
			}
			
			//sync RTC DS3231 with PC
			if(  RingState.el.RTC_DS_State == RTC_RESET
				 &&RingState.el.RTC_EMB_State == RTC_OK	)
			{
				HAL_RTC_GetDate(&hrtc, &NowDate, RTC_FORMAT_BIN);
				HAL_RTC_GetTime(&hrtc, &NowTime, RTC_FORMAT_BIN);	
				ui8_buf = NowTime.Seconds;
				while(ui8_buf == NowTime.Seconds)
				{
					HAL_RTC_GetDate(&hrtc, &NowDate, RTC_FORMAT_BIN);
					HAL_RTC_GetTime(&hrtc, &NowTime, RTC_FORMAT_BIN);
				}
				
				DS3231_SetSecond(NowTime.Seconds);
				DS3231_SetMinute(NowTime.Minutes);
				DS3231_SetHour(NowTime.Hours);
				DS3231_SetDate(NowDate.Date);
				DS3231_SetMonth(NowDate.Month);
				NowYear = NowYear - (NowYear % 100) + NowDate.Year;
				DS3231_SetYear(NowYear);
				DS3231_SetDayOfWeek(NowDate.WeekDay);
				RingState.el.RTC_DS_State = RTC_OK;
			}	
#endif

		  if(RingState.el.RTC_EMB_State == RTC_OK)
			{
				today_day = NowDate.Date; // for check of new day's begining	    
			}
		
				
			if(	RingState.el.SchedFile == SCHED_NO)
			{
				//try to find file on Flash
				HAL_FLASH_Unlock();
				
				
				// parse
				
				
				HAL_FLASH_Lock();				
			}
			
			if(RingState.el.SchedFile == SCHED_RAM)
			{
				// parse
				
				// save to Flash
	
			}
			
			
			// read parameters
			
			


			if(RingState.el.ready == RING_SETUP)
			{
				if(   RingState.el.RTC_EMB_State == RTC_OK
					 && RingState.el.SchedFile == SCHED_ON
					)
				{
					WorkMode = AUTOWORK;
						
					RingState.el.ready = RING_READY;   // setup done
				}	
			}
			
			IOState = IO_CHECK;
		}	// SETUP	
//====================================================		
		
		if(WorkMode == AUTOWORK)
		{
#ifdef MY_DEBUG	
//		HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_SET); 
//		HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_RESET);
#endif			
			
			if(today_day != NowDate.Date || RingState.el.ready != RING_READY)
			{
				WorkMode = SETUP;  // check out for new day's begining or setup wasn't finished
			}
			else
			{
			
			// check events
			
			}
			
			
	  }
//================================================================
		
		if(IOState == IO_CHECK) //only in free state - not in GET_DATA_BLOCK
		{
#ifdef MY_DEBUG	
 // 	HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_SET); 
#endif				
			if(UartReady == SET)
			{
#ifdef MY_DEBUG		
		HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_SET);
#endif					
				
				if(RMessHead.name == RING_PROT_NAME)
			  {
	  			WorkMode = REM_CONTROL;
	  			IOState = GET_COMMAND;
										
					link_time = NowTime.Seconds + NowTime.Minutes * 100;
  			}
  			else
  			{
  				memcpy(&KvHead, &RMessHead, sizeof(KvHead));
  				
  				// get trash
  				HAL_UART_Transmit_IT(&huart3, (uint8_t *)&KvHead, sizeof(KvHead)); // send it back
  				while(io_result != HAL_TIMEOUT)
  				{
  					io_result = HAL_UART_Receive(&huart3, &busy_time, 1, 1);
  					HAL_UART_Transmit_IT(&huart3, &busy_time, 1); // send it back
  				}
  				
  				UartReady = RESET;
  				memset(&RMessHead,0,sizeof(RMessHead));
  				io_result = HAL_UART_Receive_IT(&huart3, (uint8_t *)&RMessHead, sizeof(RMessHead)); // set waiting new data
  			}			
			}					
		}//IO_CHECK
//=================================================================			
	
		if(WorkMode == REM_CONTROL)
		{
			// check timeout for remote control mode	
			if(   IOState == IO_CHECK
				 && RingState.el.ready == RING_READY
				)	
			{
				// check out no link for period 1,5 minute
				busy_time = NowTime.Seconds + NowTime.Minutes * 100;
				if(busy_time > link_time)
				{
					if((busy_time - link_time) >= REMOTE_CONTROL_TIMEOUT) WorkMode = AUTOWORK;
				}
				else
				if(busy_time < link_time)	
				{
					if((busy_time + 6000 - link_time) >= REMOTE_CONTROL_TIMEOUT) WorkMode = AUTOWORK;
				}
				
			}

			
			if(IOState == GET_COMMAND)
			{
#ifdef MY_DEBUG	
		HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_SET);			
#endif				
				switch (RMessHead.index)
				{
					case RING_GET_STATE:
					{
						InitRingState(&RingStatus, RingState.all, NowTime.Hours, NowTime.Minutes, NowTime.Seconds);
						answer_index = RING_GET_STATE;
						IOState = ANSWER;
						break;
					}
					
					case RING_SET_AUTOWORK:
					{
						WorkMode = AUTOWORK;

						RingInitHead(&KvHead, RING_SET_AUTOWORK);						
						answer_index = RMessHead.index;
						IOState = ANSWER;
						break;
					}
					case RING_SET_RING: // on rele 1
					{
						HAL_GPIO_WritePin(GPIOE, Ring1_Pin, GPIO_PIN_RESET);
#ifdef MY_DEBUG	
//	HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_SET);
//	HAL_Delay(1000);					 
#endif		
						
						RingInitHead(&KvHead, RING_SET_RING);
						answer_index = RMessHead.index;
						IOState = ANSWER;
						break;
					}
					case RING_RESET_RING: // off rele 1
					{
						HAL_GPIO_WritePin(GPIOE, Ring1_Pin, GPIO_PIN_SET); 					 
#ifdef MY_DEBUG	
//	HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_RESET);			 			 
#endif
						
						RingInitHead(&KvHead, RING_RESET_RING);
						answer_index = RMessHead.index;
						IOState = ANSWER;
						break;
					}
					case RING_SET_BOOST: // on rele 2
				  {
						HAL_GPIO_WritePin(GPIOE, Ring2_Pin, GPIO_PIN_RESET);
#ifdef MY_DEBUG	
//	HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_SET);
//	HAL_Delay(1000);					 
#endif	
						RingInitHead(&KvHead, RING_SET_BOOST);
						answer_index = RING_SET_BOOST;
						IOState = ANSWER;
						break;
					}
					case RING_RESET_BOOST: // off rele 2
					{
						HAL_GPIO_WritePin(GPIOE, Ring2_Pin, GPIO_PIN_SET);
#ifdef MY_DEBUG	
//	HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_RESET);			 			 
#endif
						
						RingInitHead(&KvHead, RING_RESET_BOOST);
						answer_index = RING_RESET_BOOST;
						IOState = ANSWER;
						break;
					}
					case RING_GET_SCHEDULE:
					{
						io_result = HAL_UART_Receive(&huart3, (uint8_t*)&cod_sched_info.data, sizeof(cod_sched_info.data), 0xFFF);

						if(io_result == HAL_OK)
						{
							/// save schedule file crc32
							schedule_crc32 = cod_sched_info.data.crc32;
							
							// save schedule file size
							schedule_size = cod_sched_info.data.size;
							
							if(Schedule != NULL) free(Schedule);
							if( (Schedule = malloc(schedule_size)) == NULL ) SetRingError(&kvcod_sched_info.head);
							
							kvcod_sched_info.head.index = RING_GET_SCHEDULE;
							kvcod_sched_info.data = cod_sched_info.data;	
						}
						else
						{
							SetRingError(&kvcod_sched_info.head);
						}
						
						answer_index = RING_GET_SCHEDULE;
						IOState = ANSWER;
						
						break;
					}			
					case RING_GET_BLOCK:  // Get Schedule by blocks
					{
						io_result = HAL_UART_Receive(&huart3, (uint8_t*)&RingBlockInfo.data, sizeof(RingBlockInfo.data), 0xFFF);
						
						if(io_result == HAL_OK)	
						{
							number_block = RingBlockInfo.data.number;
							block_size = RingBlockInfo.data.size;
							block_crc32 = RingBlockInfo.data.crc32;
							
							kvRingBlockInfo.head.index = RING_GET_BLOCK;
							kvRingBlockInfo.data = RingBlockInfo.data;
						}	
						else
						{
		  		    SetRingError(&kvRingBlockInfo.head);
							block_size = 0;
						}
						
						answer_index = RING_GET_BLOCK;
						IOState = ANSWER;
						
						break;
					}
				
					case RING_END_BLOCK:  // end transmit Schedule by blocks
					{
						io_result = HAL_UART_Receive(&huart3, (uint8_t*)&RingBlockInfo.data, sizeof(RingBlockInfo.data), 0xFFF);
	
#ifdef MY_DEBUG	
//    HAL_GPIO_WritePin(GPIOE, Ring1_Pin, GPIO_PIN_RESET);					
#endif
						if(io_result == HAL_OK)
						{
							get_crc32 = CRC32(Schedule, schedule_size);
							
							kvEndBlockInfo.head.index = RING_END_BLOCK;
							kvEndBlockInfo.data.size = schedule_size;
							kvEndBlockInfo.data.crc32 = get_crc32;
							
#ifdef MY_DEBUG	
 //   HAL_GPIO_WritePin(GPIOE, Ring2_Pin, GPIO_PIN_RESET);					
#endif
							
							if(get_crc32 == schedule_crc32)
							{
								RingState.el.SchedFile = SCHED_RAM;
								WorkMode = SETUP; // go to parse
							}
						}						
						else
						{
						  SetRingError(&kvEndBlockInfo.head);
						}
						
						answer_index = RING_END_BLOCK;
						IOState = ANSWER;
						
						break;
					}
					case RING_ON_TRANSFER:
					{		
						InitRingTransfer(&RingStatus, 0xA, 0xB, 0xC, 0xD);
						WorkMode = TRANSIT_DATA;
						
						busy_time = NowTime.Seconds;
						
										
						answer_index = RING_ON_TRANSFER;
						IOState = ANSWER;				
						break;
					}
					case RING_SYNC_TIME:
					{
						io_result = HAL_UART_Receive(&huart3, (uint8_t*)&PCSyncTime.data, sizeof(PCSyncTime.data), 0xFFF);

#ifdef MY_DEBUG	
//	  HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_SET);	
#endif						
						if(io_result == HAL_OK)
						{							
							if(PCSyncTime.data.data1 == 0) // sync seconds
							{
								PCTime.Seconds	= PCSyncTime.data.data4;
								InitRingSyncTime(&kvPCSyncTime, 0, 0, 0, PCTime.Seconds);
							}
							else
							if(PCSyncTime.data.data1 == 1) // sync time
							{
								PCTime.Hours = PCSyncTime.data.data2;
								PCTime.Minutes = PCSyncTime.data.data3;
								PCDate.WeekDay = PCSyncTime.data.data4;
								
								HAL_RTC_SetTime(&hrtc, &PCTime, RTC_FORMAT_BIN);
																
								InitRingSyncTime(&kvPCSyncTime, PCTime.Hours, PCTime.Minutes, PCTime.Seconds, PCDate.WeekDay);
						  }
							else
							if(PCSyncTime.data.data1 == 2) // sync date
							{
								NowYear = PCSyncTime.data.data2;
								PCDate.Year = NowYear % 100;
								PCDate.Month = PCSyncTime.data.data3;
								PCDate.Date = PCSyncTime.data.data4;
								
								HAL_RTC_SetDate(&hrtc, &PCDate, RTC_FORMAT_BIN);
								
								InitRingSyncTime(&kvPCSyncTime, PCDate.Year, PCDate.Month, PCDate.Date, PCDate.WeekDay);
								
								RingState.el.RTC_EMB_State = RTC_OK;
								if(RingState.el.RTC_DS_State == RTC_OK) RingState.el.RTC_DS_State = RTC_RESET;
								
								WorkMode = SETUP;
							}														
					
						answer_index = RING_SYNC_TIME;
						IOState = ANSWER;							

							
						}
						else
						{
		  		    SetRingError(&kvPCSyncTime.head);
							answer_index = RING_SYNC_TIME;
							IOState = ANSWER;		
						}
										
						break;
					}
					
				}//switch
			}//IOState Command mode
//======================================================================			

		  if(IOState == ANSWER)
			{
				switch (answer_index)
				{
					case RING_SET_AUTOWORK:
					case RING_SET_RING:
					case RING_RESET_RING:
					case RING_SET_BOOST:
					case RING_RESET_BOOST:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&KvHead, sizeof(KvHead)); // send it back
						break;
					}
					case RING_GET_SCHEDULE:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&kvcod_sched_info, sizeof(kvcod_sched_info)); // send it back
						break;
					}
					case RING_GET_BLOCK:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&kvRingBlockInfo, sizeof(kvRingBlockInfo)); // send it back 
						
						if(block_size > 0)
						{
							IOState = GET_DATA_BLOCK;
							
							UartReady = RESET;
							io_result = HAL_UART_Receive_IT(&huart3, &buf_head, 1); // set waiting new data
						}
						
						break;
					}
					case RING_KV_BLOCK:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&kvRingBlockInfo, sizeof(kvRingBlockInfo)); // send it back
						break;
					}
					case RING_END_BLOCK:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&kvEndBlockInfo, sizeof(kvEndBlockInfo)); // send it back 
						break;
					}
					case RING_ON_TRANSFER:
					case RING_GET_STATE:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&RingStatus, sizeof(RingStatus)); // send it back 
						break;
					}
					case RING_SYNC_TIME:
					{
						HAL_UART_Transmit_IT(&huart3, (uint8_t *)&kvPCSyncTime, sizeof(kvPCSyncTime)); // send it back
						break;
					}
				}// switch
				
				if(IOState == ANSWER)
				{
#ifdef MY_DEBUG	
	  HAL_GPIO_WritePin(GPIOB, LD2B_Pin|LD3R_Pin, GPIO_PIN_RESET);	
#endif
					UartReady = RESET;
					memset(&RMessHead,0,sizeof(RMessHead));
					io_result = HAL_UART_Receive_IT(&huart3, (uint8_t *)&RMessHead, sizeof(RMessHead)); // set waiting new data				
					
					IOState = IO_CHECK;
					answer_index = RING_IDLE_DATA;
				}	
			}// ANSWER
//=======================================================================	
				
		  if(IOState == GET_DATA_BLOCK)
			{
				// checkout control line
				if(UartReady == SET) // we got something from UART
				{
					io_result = HAL_UART_Receive(&huart3, &io_buf[1], block_size-1, 0xFFF);
					
					if(io_result == HAL_OK)
					{	
						io_buf[0] = buf_head;
						
						get_crc32 = CRC32((unsigned char*)&io_buf, block_size);
						
						InitBlockInfo(&kvRingBlockInfo, number_block, block_size, get_crc32);
						
						if(get_crc32 == block_crc32)
						{
							if(   ((IO_BLOCK_SIZE*(number_block-1)) <= schedule_size)
								 && (number_block > 0)
								)
							{
								memcpy((Schedule + (IO_BLOCK_SIZE*(number_block-1))), io_buf, block_size);
							}
						}
						else
						{
							SetRingError(&kvRingBlockInfo.head);
						}
					}	
					else
					{
						InitBlockInfo(&kvRingBlockInfo,number_block , 0, 0); 
						SetRingError(&kvRingBlockInfo.head);
					}
					
					answer_index = RING_KV_BLOCK;
					IOState = ANSWER;
					
				}// if got UART
			}//GET_DATA_BLOCK
			
	  }// WorkMode = REM_CONTROL
//======================================================================
		
		if(WorkMode == TRANSIT_DATA)
		{
			if(busy_time != NowTime.Seconds)
			{
			
				if(ui_flag == 0)
				{
				  InitRingTransfer(&RingStatus, 0xA, NowTime.Hours, NowTime.Minutes, NowTime.Seconds);
					ui_flag = 1;
				}
				else
				{
#ifdef USE_DS3231					
					InitRingTransfer(&RingStatus, 0xB, DS3231_GetHour(), DS3231_GetMinute(), DS3231_GetSecond());
#else
					InitRingTransfer(&RingStatus, 0xA, NowTime.Hours, NowTime.Minutes, NowTime.Seconds);
#endif					
					ui_flag = 0;
				}
				
				HAL_UART_Transmit_IT(&huart3, (uint8_t *)&RingStatus, sizeof(RingStatus)); // send it back 				
				
				
				busy_time = NowTime.Seconds;
			}
			
		}// TRANSIT_DATA
//======================================================================		
		

		
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#ifdef MY_DEBUG		
//		HAL_Delay(1000);
//	  HAL_GPIO_WritePin(GPIOB, LD2B_Pin|LD3R_Pin, GPIO_PIN_RESET);
//		HAL_Delay(200);		
//	  HAL_GPIO_WritePin(GPIOB, LD2B_Pin, GPIO_PIN_RESET);		
#endif		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10B0DCFB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_POS1;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable Calibrartion
  */
  if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 9;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, Ring2_Pin|Ring1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3R_Pin|LD2B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Ring2_Pin Ring1_Pin */
  GPIO_InitStruct.Pin = Ring2_Pin|Ring1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3R_Pin LD2B_Pin */
  GPIO_InitStruct.Pin = LD3R_Pin|LD2B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

	int SigTime = 0xE;

	while(SigTime)		
  {
   HAL_GPIO_TogglePin(GPIOB, LD3R_Pin);
   HAL_Delay(200);

	 --SigTime;
	}	
	HAL_GPIO_WritePin(GPIOB, LD3R_Pin, GPIO_PIN_RESET);
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
