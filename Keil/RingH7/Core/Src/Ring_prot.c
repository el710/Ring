#include "Ring_prot.h"


int SetIdleData(void * data, unsigned int size)
{
 for( int i=0; i<size; i++)  *((char*)data) = RING_IDLE_DATA;
 return 0;
}
//------------------------------------------

int RingInitHead(T_RingIO_Head * head, unsigned short in_index)
{
	head->name = RING_PROT_NAME;
	head->index = in_index;
	return 0;
}
//------------------------------------------

int InitRingOnCom(T_RingIO_Head * code)
{
 code->name = RING_PROT_NAME;
 code->index = RING_SET_RING;
 return 0;
}
//------------------------------------------

int InitRingOffCom(T_RingIO_Head * code)
{
 code->name = RING_PROT_NAME;
 code->index = RING_RESET_RING;
 return 0;
}
//------------------------------------------

int InitBoostOnCom(T_RingIO_Head * code)
{
 code->name = RING_PROT_NAME;
 code->index = RING_SET_BOOST;
 return 0;
}
//------------------------------------------

int InitBoostOffCom(T_RingIO_Head * code)
{
 code->name = RING_PROT_NAME;
 code->index = RING_RESET_BOOST;
 return 0;
}
//------------------------------------------




int SetRingError(T_RingIO_Head * code)
{
	code->index = (code->index | RING_ERROR_REPORT) & 0xFFFF;
	return 0;
}
//------------------------------------------

int InitExchSchedCom(T_RingIO_SchData * code, unsigned int size, unsigned int sum)
{
 code->head.name = RING_PROT_NAME;
 code->head.index = RING_GET_SCHEDULE;
 code->data.size = size;	
 code->data.crc32 = sum;
 return 0;
}
//------------------------------------------

int InitSchedIORes(T_RingIO_Head * code, unsigned short res)
{
 code->name = RING_PROT_NAME;
 code->index = res;
 return 0;	
}
//---------------------------------------------

int InitBlockInfo(T_RingIO_BlockInfo *code, unsigned short in_num, unsigned short in_size, unsigned int in_sum)
{
 code->head.name = RING_PROT_NAME;
 code->head.index = RING_GET_BLOCK;
 code->data.number = in_num;
 code->data.size = in_size;
 code->data.crc32 = in_sum;
 return 0;
}
//------------------------------------------

int InitRingEndSched(T_RingIO_BlockInfo *code, unsigned short in_num, unsigned short in_size, unsigned int in_sum)
{
 code->head.name = RING_PROT_NAME;
 code->head.index = RING_END_BLOCK;
 code->data.number = in_num;
 code->data.size = in_size;
 code->data.crc32 = in_sum;
 return 0;
}	
//------------------------------------------

int InitRingTransfer(T_RingIO_Status * code, unsigned int in_data1, unsigned int in_data2, unsigned int in_data3, unsigned int in_data4)
{
  code->head.name = RING_PROT_NAME;
  code->head.index = RING_ON_TRANSFER;
  code->data.data1 = in_data1;
  code->data.data2 = in_data2;
	code->data.data3 = in_data3;
	code->data.data4 = in_data4;
  return 0;
}
//-------------------------------------------

int InitRingState(T_RingIO_Status * code, unsigned int in_data1, unsigned int in_data2, unsigned int in_data3, unsigned int in_data4)
{
  code->head.name = RING_PROT_NAME;
  code->head.index = RING_GET_STATE;
  code->data.data1 = in_data1;
  code->data.data2 = in_data2;
	code->data.data3 = in_data3;
	code->data.data4 = in_data4;
  return 0;
}
//-------------------------------------------

int InitRingSyncTime(T_RingIO_Status * code, unsigned int in_data1,
                                             unsigned int in_data2,
                                             unsigned int in_data3,
                                             unsigned int in_data4)
{
  code->head.name = RING_PROT_NAME;
  code->head.index = RING_SYNC_TIME;
  code->data.data1 = in_data1;
  code->data.data2 = in_data2;
  code->data.data3 = in_data3;
  code->data.data4 = in_data4;
  return 0;
}
//-------------------------------------------