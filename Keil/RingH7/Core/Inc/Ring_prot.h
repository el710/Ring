#ifndef RING_PROTOCOL_H
#define RING_PROTOCOL_H

#define IO_BLOCK_SIZE       128


#define RING_IDLE_DATA      0x55
#define RING_ERROR_REPORT   0xEE00


#define RING_PROT_NAME      0x7B

#define RING_RESET_RING     0x31
#define RING_SET_RING       0x32
#define RING_RESET_BOOST    0x33
#define RING_SET_BOOST      0x34
#define RING_GET_SCHEDULE   0x35
#define RING_SET_AUTOWORK   0x36

#define RING_GET_BLOCK      0x37
#define RING_END_BLOCK      0x38
#define RING_KV_BLOCK       0x39

#define RING_GET_STATE      0x3A
#define RING_ON_TRANSFER    0x3B
#define RING_SYNC_TIME      0x3C



//===============================
// types

typedef struct
{
 unsigned short name;
 unsigned short index;
} T_RingIO_Head;

typedef struct
{
 T_RingIO_Head head;
 struct
 {
   unsigned int size;
   unsigned int crc32;
 }data;
} T_RingIO_SchData;

typedef struct
{
 T_RingIO_Head head;
 struct
 {
   unsigned short number;
   unsigned short size;
   unsigned int crc32;
 }data;

} T_RingIO_BlockInfo;

typedef struct
{
 T_RingIO_Head head;
 struct
 {
   unsigned int data1;
   unsigned int data2;
   unsigned int data3;
   unsigned int data4;
 }data;

} T_RingIO_Status;



//===============================
// function declarations

int MyIdle();

int SetIdleData(void* data, unsigned int size);

int RingInitHead(T_RingIO_Head* head, unsigned short in_index);

int InitRingOnCom(T_RingIO_Head * code);
int InitRingOffCom(T_RingIO_Head * code);

int InitBoostOnCom(T_RingIO_Head * code);
int InitBoostOffCom(T_RingIO_Head * code);


int InitExchSchedCom(T_RingIO_SchData * code, unsigned int size, unsigned int sum);

int InitSchedIORes(T_RingIO_Head * code, unsigned short res);

int SetRingError(T_RingIO_Head * code);

int InitExchSched(T_RingIO_SchData * code, unsigned int in_block, unsigned int in_sum);

int InitBlockInfo(T_RingIO_BlockInfo *code, unsigned short in_num, unsigned short in_size, unsigned int in_sum);
int InitRingEndSched(T_RingIO_BlockInfo *code, unsigned short in_num, unsigned short in_size, unsigned int in_sum);

int InitRingTransfer(T_RingIO_Status * code, unsigned int in_data1,
                                             unsigned int in_data2,
                                             unsigned int in_data3,
                                             unsigned int in_data4);
																						 
int InitRingState(T_RingIO_Status * code, unsigned int in_data1, unsigned int in_data2, unsigned int in_data3, unsigned int in_data4);
	
int InitRingSyncTime(T_RingIO_Status * code, unsigned int in_data1,
                                             unsigned int in_data2,
                                             unsigned int in_data3,
                                             unsigned int in_data4);
#endif // RING_PROTOCOL_H
