/* Calculate function Crc32 
   Using polinom EDB88320UL = x^32 + x^26 + x^23 + x^22 + x^16 
                                     + x^12 + x^11 + x^10 + x^8 + x^7 
                                     + x^5 + x^4 + x^2 + x + 1
*/

#include "crc32.h"
	

unsigned int CRC32(unsigned char *buf, unsigned int len)
{
 unsigned int crc_table[256];
 unsigned int crc;

 for (unsigned int i = 0; i < 256; i++)
    { 
     crc = i;
 
     for (unsigned int j = 0; j < 8; j++)
        crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

     crc_table[i] = crc;
    }

 crc = 0xFFFFFFFFUL;

 while (len--)
   crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

 return crc ^ 0xFFFFFFFFUL; 
};
 


/* Example

int main()
{
 char buf[4096];
 int len;
 
 FILE *f = fopen("text.txt", "rt");//??????? ???? ??? ??????
 
 if ((f = fopen("text.txt", "rt"))==NULL)//???? ???? ?????????? ???????,
   {
    printf ("Error = Nevozmozchno otcrjt' fail");//?? ?????? ?????????
    getch();
    return 0;//? ??????? ?? ????????? 
   }

 fgets(buf, sizeof(buf), f);
 len = strlen(buf);
 
 //????? Crc32
 printf("CRC-32 = %p\n", Crc32(buf,len));//??? MVisual ?????? ??????
 
 fclose(f);//???? ???????

 return 0;
}
*/