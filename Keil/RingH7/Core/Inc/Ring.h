#ifndef RING_H
#define RING_H




#define MY_DEBUG

#define USE_DS3231

//-----------------------------------------------------

//---------------- USE FLASH
#define DATA_FLASH  0x08108000
#define SCHEDULE_SIZE_ADDR   DATA_FLASH
#define SCHEDULE_DATA_ADDR   (DATA_FLASH + 0x8)
//-------------------------------------------------------

//---------------- WORK MODE 
typedef enum {SETUP, AUTOWORK, REM_CONTROL, TRANSIT_DATA, SYNC_TIME} T_WorkState;

typedef enum {IO_CHECK, GET_COMMAND, ANSWER, GET_DATA_BLOCK, WAIT_SCHEDULE} T_IOState;

#define REMOTE_CONTROL_TIMEOUT  130 /// min+sec in dec

//---------------------------------------------------------

//---------------- STATE
typedef union
{
	unsigned int all;
	struct
	{
		unsigned int RTC_EMB_State: 8;
		unsigned int RTC_DS_State: 8;
		unsigned int SchedFile: 8;
		unsigned int ready: 8;			
	}el;
} T_RINGState;

//======= for T_RINGState =======
#define RTC_OK          0  // RTC is ready
#define RTC_RESET       1  // RTC works but reset
#define RTC_NO          2  // RTC not found
#define RTC_UNDEF       3  // unknown state

#define SCHED_NO        0  // no schedule file
#define SCHED_IS        1  // schedule file on flash
#define SCHED_RAM       2  // schedule file on RAM - gotten from PC
#define SCHED_ON        3  // schedule ready for work
#define SCHED_ERR       4  // schedule can not be setup

#define RING_SETUP      0 // setup RTC & Schedule
#define RING_READY      1 // setup done
//----------------------------------------------------------



#endif // RING_H

