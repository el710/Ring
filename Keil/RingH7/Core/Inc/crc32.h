#ifndef CRC32_H
#define CRC32_H

#define POLINOM 0xEDB88320UL 

/* Calculate function Crc32 
   Using polinom EDB88320UL = x^32 + x^26 + x^23 + x^22 + x^16 
                                     + x^12 + x^11 + x^10 + x^8 + x^7 
                                     + x^5 + x^4 + x^2 + x + 1
*/

unsigned int CRC32(unsigned char *buf, unsigned int len);


#endif // CRC32_H