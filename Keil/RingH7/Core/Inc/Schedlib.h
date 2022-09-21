#pragma once
#ifndef SHEDULE_LIB_HEADER
#define SHEDULE_LIB_HEADER

#define STM_USAGE


#ifdef STM_USAGE
   typedef unsigned char bool;
	 #define false 0
	 #define true  1
#endif


//======== list of function's results
#define  EL_RES_NO_EVENT        -1
#define  EL_RES_OK              0
#define  EL_RES_BASE_EMPTY      1
#define  EL_RES_NULL_POINTER    2
#define  EL_RES_ALLOCMEM_ERROR  3
#define  EL_RES_NO_FILE         4
#define  EL_RES_FILE_EMPTY      5


//======== Scaling
#define FILE_STRING_SIZE_MAX  256
#define FILE_PARAMETERS_COUNT 18

#define FILE_STRING_END_CHAR  0x0A
#define FILE_RECORD_SEPARATOR 0x23
#define FILE_PARAM_SEPARATOR  0x20
#define ASCII_NUMBER_BASE     0x30

#define MESSAGE_ROW_SIZE      80
#define SOUND_FILENAME_SIZE   80


//============ Event parameters & types
#define WD_MONDAY   0x1
#define WD_TUESDAY  0x2
#define WD_WEDNSDAY 0x4
#define WD_THURSDAY 0x8
#define WD_FRIDAY   0x10
#define WD_SATURDAY 0x20
#define WD_SUNDAY   0x40


typedef struct
{
 unsigned year    :16;  // 0x270F
 unsigned month   :4;   // 0xC
 unsigned day     :5;   // 0x1F
 unsigned weekday :7;   //  WD_MONDAY +... + WD_SUNDAY
}s_Date;

typedef struct
{
 unsigned hour   :5;  // 0x18
 unsigned minute :6;
 unsigned second :6;
}s_Time;

typedef struct
{
 unsigned year    :16;  // 0x270F
 unsigned month   :4;   // 0xC
 unsigned day     :5;   // 0x1F
 unsigned weekday :7;   //  WD_MONDAY +... + WD_SUNDAY

 bool	use_year:1;
 bool use_month:1;
 bool use_day:1;
 bool use_weekly:1;
 bool exclusive_day:1;     // this date is exclusive - for Example New Year Day

}T_Date;

typedef struct
{
 bool use_hour:1;
 bool use_minute:1;
 bool use_second:1;

 unsigned hour   :5;  // 0x18
 unsigned minute :6;
 unsigned second :6;
}T_Time;


typedef struct
{
 T_Date start_date;

 T_Time start_time;

 union
   {
    unsigned int all_flags;
    struct
      {
       int id_event        : 21;
       int id_rfile        : 8;
       unsigned is_event_active : 1;    // active / hide
       unsigned is_showed : 1;
       unsigned even_in_exclusive : 1;  // show event in exclusive days
      }el;
   }cycle_flags;

 union
   {
    unsigned int all_signs;
    struct
      {
       unsigned ring_time :8; // in seconds
       unsigned is_ring : 1;
       unsigned is_sound : 1;
       unsigned is_text : 1;
      }el;
   }event_sign;

 char* caption;
 char* sound;

}T_Event;


typedef struct T_ListItem
{
  T_Event event;

  struct T_ListItem* prev;
  struct T_ListItem* next;
} T_ListItem;



typedef struct
{
  T_ListItem* first;
  T_ListItem* last;

  unsigned int size;
} T_EventList;


//========= defines of functions

int NewEvent(T_Event * event);  // make event structure
int FreeEvent(T_Event * event); // free allocated memory
void InitEvent(T_Event * event); // init event structure

void EventList_init(T_EventList* list);  // init EventList variables
void EventList_clear(T_EventList* id_list); // free items of EventList
T_ListItem* EventList_add(T_EventList* id_list, T_Event* in_event);  // add new Item to List & increase item number
T_ListItem* EventList_copy(T_EventList* id_list, T_Event* in_event);  // copy Item from list to List & save item number
T_ListItem* EventList_get(T_EventList* id_list, unsigned int in_num);  // find event in List by number
T_ListItem* EventList_update(T_EventList* id_list, unsigned int in_num, T_Event* in_event);
void EventList_delete(T_EventList* id_list, unsigned int in_num);


int EventList_ReadMem(char* in_memfile, unsigned int in_size, T_EventList* out_list);

int EventList_ReadFile(const char* in_SchedFile, T_EventList* out_base);
int EventList_SaveFile();

int MakeTodaySchedule(T_EventList* in_base, s_Date* in_date,
                      T_EventList* out_today, bool* out_holyday);
T_ListItem* CheckSchedule(T_EventList* id_list, s_Time* in_time);


#endif // SHEDULE_LIB_HEADER