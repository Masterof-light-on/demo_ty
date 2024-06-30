#ifndef __USR_PUBLIC_H__
#define	__USR_PUBLIC_H__

#include "usr_mile.h"

#define ABS(a,b) 		((a)>=(b)?((a)-(b)):(b)-(a))	//求差的绝对值
#define CHAR2DEC(x)   	((x)-'0')
#define CHAR2HEX(x)   	((((x)>'9')?((x)-'A'+10):((x)-'0'))&0xF)

#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define islower(c)           in_range(c, 'a', 'z')

#define LOG_DISABLE  0		/* 不输出调试信息 */ 
#define LOG_RUNINFO  0x01	/* 输出调试信息 */
#define LOG_GPSINFO	 0x02	/* 开启GPS报文 */
#define LOG_CANINFO	 0x03	/* 开启CAN报文 */

#define LOG(...)	  do{\
						if((sys_get_debugSwitch()==(LOG_RUNINFO))||(sys_get_debugSwitch()==(LOG_CANINFO))){ \
							Log_Lock(xLogMutex);\
							PRINTF(__VA_ARGS__); \
							Log_UnLock(xLogMutex);}\
						}while(0)

#define LOGX(message,len) do{\
							if((sys_get_debugSwitch()==(LOG_RUNINFO))||(sys_get_debugSwitch()==(LOG_CANINFO))){ \
								Log_Lock(xLogMutex);\
								printx(message,len); \
								Log_UnLock(xLogMutex);}\
							}while(0)

#define LOG_EXT(...)	do{\
							if(sys_get_debugSwitch()==(LOG_CANINFO)){ \
								Log_Lock(xLogMutex);\
								PRINTF(__VA_ARGS__); \
								Log_UnLock(xLogMutex);}\
							}while(0)
	
#define LOGX_EXT(message,len) do{\
								if(sys_get_debugSwitch()==(LOG_CANINFO)){ \
									Log_Lock(xLogMutex);\
									printx(message,len); \
									Log_UnLock(xLogMutex);}\
								}while(0)

void printx(unsigned char *msg,unsigned short len);
void log_Init(void);
int uint32_to_uint8(unsigned int val,unsigned char *msg);
int uint16_to_uint8(unsigned short val,unsigned char *msg);
unsigned short uint8_to_uint16(unsigned char *msg);
unsigned int uint8_to_uint32(unsigned char *msg);
float myatof(char *str);
unsigned int myatoi(char* pdata);
unsigned char Is_Digit(char* pdata,unsigned char len);
int is_ip_address(const char *pVal, unsigned char uLen);
int rtc_check_time(TIME *Time0, TIME *Time1);
unsigned short getAngle(char* plast,char* pnow);
unsigned char HexToCompactBCD(unsigned char pchr);
unsigned int HexString2Dword(const char *pstr);
int GetItemCount(char *src_string, char item);
int GetStringPartEx(const char **pSrc, char *pBuf, int len, char ch);
int GetStringPart(const char **pSrc, char *pBuf, int len);
void floatStr_to_bcd_exe(unsigned char *outData, unsigned int outlen, char *str);
void floatStr_to_bcd(unsigned char *outData, unsigned int outlen, char *str);
void Byte2String(const unsigned char* pSrc,unsigned char* pDst,unsigned short nSrcLength);
void String2Byte(const unsigned char* pSrc,unsigned char* pDst,unsigned short nSrcLength);
unsigned int myhtonl(unsigned int n);
void latlon_to_degree(char* slat0,char*slon0,Point *pt);
unsigned short memcmp_inverted(char *buf1, char *buf2);
char *strchr_inverted(char const *s1, int s2);
int mystrncmp (const char *s1, const char *s2, int n1, int n2);
char *mystrstr(char *pOri, int OriNum, char *pFind, int FindNum);
int bytepos(const unsigned char* pSrc, unsigned short nSrc,const char* pSub, unsigned short startPos);
void parse_lat_or_lon_to_double(char *str, double *ret);
unsigned char Toupper(unsigned char c);
char *f2s(double f, int bit, char *out);
unsigned short usc2_ascii(unsigned char *pSrc, unsigned char *pDst, unsigned short nSrcLength);
unsigned char xor_check(unsigned char *data, unsigned int len);
#endif
