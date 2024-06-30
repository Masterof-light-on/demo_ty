#include "includes.h"

//Log初始化
void log_Init(void)
{
	usb_init();
}

//打印16进制的数据
void printx(unsigned char *msg,unsigned short len)
{
	unsigned short i=0;
	
	for(i=0;i<len;i++)
	{
		PRINTF("%02X",msg[i]);
	}
	PRINTF("\r\n");
}

int uint32_to_uint8(unsigned int val,unsigned char *msg)
{
	msg[0] = val>>24;
	msg[1] = val>>16;
	msg[2] = val>>8;
	msg[3] = val&0xFF;

	return 4;
}
int uint16_to_uint8(unsigned short val,unsigned char *msg)
{
	msg[0] = val>>8;
	msg[1] = val&0xFF;

	return 2;
}
//返回一个16位值
unsigned short uint8_to_uint16(unsigned char *msg)
{
	unsigned short val = msg[0];
	
	val <<= 8;
	return (val|msg[1]);
}

//返回一个32位值
unsigned int uint8_to_uint32(unsigned char *msg)
{
	unsigned int val;

	val = uint8_to_uint16(&msg[0]);
	val <<= 16;
	val += uint8_to_uint16(&msg[2]);

	return val;
}

int pow_my(int base,int n)
{    
    int result=1;
	
    while(n-->0){
		
        result *= base;
    }
	
    return result;
}

float myatof(char *str)
{
    int i=0,j=0;
    int sum=0;
    int flag=0;
    int len=strlen_t(str);
    float decimal=0.0;

    for(i=0;i<len;i++,str++)
	{
	    if(*str=='.') {flag=1;continue;}
	    if(*str!='.'&&flag==0){
			
		    sum = sum * 10+(*str-'0');
	    }
	    
	    if(flag==1){

		    j++;
		    decimal += (float)(*str-'0')/pow_my(10,j);
	    }
	    
    }
    return sum+decimal;
}

unsigned int myatoi(char* pdata)
{
	unsigned char i;
	unsigned int n=0;
	
	for(i=0;i<10 && (*pdata>='0' && *pdata<='9');i++)
	{
		n *= 10;
		n +=(*pdata - 0x30);
		pdata++;
	}
	return n;
}

//判断是否是数字
unsigned char Is_Digit(char* pdata,unsigned char len)
{
	unsigned char i=0;
	
	for(i=0;i<len;i++)
	{
		if((pdata[i] >'9') || (pdata[i] <'0'))
		{
			return 0;
		}
	}
	return 1;
}

/*判断输入的字符串是否是ip的格式*/
int is_ip_address(const char *pVal, unsigned char uLen)
{
	unsigned char i;
	unsigned char uCnt = 0;

	if((uLen>=7) && (uLen<16))
	{
		for(i=0; i<uLen; i++)
		{
			if(pVal[i] == '.')
			{
				uCnt++;
			}
			else if((pVal[i]>='0') && (pVal[i]<='9'))
			{
				continue;
			}
			else
			{
				return -1;
			}
		}
		if(3 == uCnt)
		{
			return 0;
		}
	}

	return -1;
}

unsigned int HexString2Dword(const char *pstr)
{
	unsigned int i=0;
	unsigned int len=strlen_t(pstr);
	unsigned int ret=0;
	
	for(i=0; i<len; i++,pstr++)
	{
		ret <<= 4;
		ret |= CHAR2HEX(*pstr);
	}

	return ret;
}

int GetItemCount(char *src_string, char item)
{
	unsigned int SentenceCnt = 0;
	unsigned int i,len = 0;
	
	len = strlen_t(src_string);
	for(i = 0; i < len; i++)
	{
		if(*(src_string + i) == item)//','
		{
			SentenceCnt++;
		}
	}
	
	return SentenceCnt;
}

int GetStringPartEx(const char **pSrc, char *pBuf, int len, char ch)
{
	int		i;
	
	i = 0;
	while (i < len)
	{
		if ((**pSrc == ch) || (**pSrc == 0))
			break;
		*pBuf++ = **pSrc;
		(*pSrc)++;
		i++;
	}
	*pBuf = 0;
	return i;
}

int GetStringPart(const char **pSrc, char *pBuf, int len)
{
	return GetStringPartEx(pSrc, pBuf, len, ',');
}

//时间差计算
int rtc_check_time(TIME *Time0, TIME *Time1) 
{
	int t0=0, t1=0, t24=0;

	t0 = Time0->hour*3600 + Time0->minute*60 + Time0->second;
	t1 = Time1->hour*3600 + Time1->minute*60 + Time1->second;
	t24 = 24*3600 + 0*60 + 0; 
	if (t1 > t0)
		return (t1 - t0);
	else if(t1 == t0)
		return 0;
	else
		return ((t24 - t0) + t1);
}


//拐点计算
unsigned short getAngle(char* plast,char* pnow)
{
	unsigned short nl=0,nn=0;

	nl = myatoi(pnow);
	nn = myatoi(plast);

	if(nn > nl)
	{
		if((nn-nl) > 180)
		{
			return 360-(nn-nl);
		}
		else
		{
			return (nn-nl);
		}
	}
	else
	{
		if((nl-nn) > 180)
		{
			return 360-(nl-nn);
		}
		else
		{
			return (nl-nn);
		}
	}
}

unsigned char HexToCompactBCD(unsigned char pchr)
{
	unsigned char chr;

	chr = ((pchr / 10) << 4) + (pchr % 10);

	return chr;
}
void floatStr_to_bcd_exe(unsigned char *outData, unsigned int outlen, char *str)
{
	int i=0, j=0;
	bool trigger = TRUE;
	
	while(j<outlen)
	{
		if(str[i] == '.')//skip '.'s
			i++;
		
		if(trigger)
		{
			outData[j] |= (str[i]-0x30)<<4;
		}
		else
		{			
			outData[j] |= (str[i]-0x30);
			j++;
		}
		i++;
		trigger = !trigger;
	}
}
void floatStr_to_bcd(unsigned char *outData, unsigned int outlen, char *str)
{
	int i=0, j=0;
	bool trigger = TRUE;
	char *dotP = NULL;

	dotP = strchr_t(str, '.');
	if(dotP)
	{
		i = (unsigned int)(dotP - str) - 1;
	}
	else
	{
		i = strlen_t(str) - 1;
	}

	j = outlen - 1;

	while(i>=0 && j>=0)
	{
		if(trigger)
		{
			outData[j] |= (str[i]-0x30);
		}
		else
		{
			outData[j] |= (str[i]-0x30)<<4;
			j--;
		}
		i--;
		trigger = !trigger;
	}
}
void Byte2String(const unsigned char* pSrc,unsigned char* pDst,unsigned short nSrcLength)
{
	unsigned short i=0;
	unsigned char ch=0;
	
	for(i=0;i<nSrcLength;i++)
	{

		ch = (*pSrc >> 4)&0x0F;

		if(ch >= 0x0A)
			*pDst++ = ch + 'A'-0x0A;
		else
			*pDst++ = ch + '0';

		ch = *pSrc&0x0F;

		if(ch >= 0x0A)
			*pDst++ = ch + 'A'-0x0A;
		else
			*pDst++ = ch + '0';
		pSrc++;
	}
	*pDst = 0x00;
}

void String2Byte(const unsigned char* pSrc,unsigned char* pDst,unsigned short nSrcLength)
{
	unsigned short i;

	for(i=0; i<nSrcLength; i+=2)
	{
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		
		pSrc++;
		
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		
		pSrc++;
		pDst++;
	}
	
	return;
}

unsigned int myhtonl(unsigned int n)
{
	unsigned char ch =0;
	unsigned char* p = (unsigned char*)&n;

	ch = p[0];
	p[0] = p[3];
	p[3] = ch;
	ch = p[1];
	p[1] = p[2];
	p[2] = ch;

	return n;
}

void latlon_to_degree(char* slat0,char*slon0,Point *pt)
{
	char ddd[4] = {0}, mm_mmmm[8]={0};

	memcpy_t(ddd, slat0,3);
	memcpy_t(mm_mmmm, slat0+3,7);
	pt->x = myatoi(ddd) + myatof(mm_mmmm) / 60;

	memcpy_t(ddd, slon0, 3);
	memcpy_t(mm_mmmm, slon0+3,7);
	pt->y = myatoi(ddd) + myatof(mm_mmmm) / 60;
}

unsigned short memcmp_inverted(char *buf1, char *buf2)
{
	unsigned short i=0,j=0;
	
	if((strlen_t((char*)buf1) == 0)||(strlen_t((char*)buf2) == 0))
	{
		return 1;
	}
	i = strlen_t((char*)buf2);
	j = strlen_t((char*)buf1);
	
	for(;(i>0)&&(j>0);i--,j--)
	{
		if(buf1[j-1] != buf2[i-1])
		{
			return 1;
		}
	}
	return 0;
}

//查找最后位置的字符
char *strchr_inverted(char const *s1, int s2)
{
	char *last = NULL;
	char *current = NULL;

	if(s2 != '\0')
	{
		current = strchr_t(s1, s2);
		while(current != NULL)
		{
			last = current;
			current = strchr_t(last+1, s2);
		}
	}

	return last;
}

//匹配字符,过滤字符前面出现的0x00
int mystrncmp (const char *s1, const char *s2, int n1, int n2)
{
	/* No checks for NULL */
	char *s1p = (char *)s1;
	char *s2p = (char *)s2;
	bool bFindStart = FALSE;

	if ((n1 <= 0) || (n2 <= 0))
		return -1;

	while (n1--)
	{
		if (*s1p != *s2p)
		{
			if(bFindStart == TRUE)/* 回溯到前面一个 */
			{
				++n2;
				--s2p;
				bFindStart = FALSE;
			}
			++s1p;
			continue;
		}

		if (--n2 == 0)
			break;

		if (n1 == 0)
			break;

		++s1p;
		++s2p;
		bFindStart = TRUE;/* 开始连续的查找 */
	}
	return ((*s1p - *s2p) | n2);
}

//可跳过0查找函数
char *mystrstr(char *pOri, int OriNum, char *pFind, int FindNum)
{
	char *p = NULL;
	int i = 0, j = 0, Match = 0;
	int e = 0;
	
	if(OriNum < FindNum)
	{
		return NULL;
	}
	else
	{
		for(i = 0; i < OriNum && FindNum + i + 1 <= OriNum; i++)
		{
			e = i;
			for(j = 0; j < FindNum; j++)
			{
				if(!memcmp_t(pFind + j, pOri + e, 1))
				{
					e++;
					Match++;
				}
				else
				{
					Match = 0;
				}
			}
			if(Match == FindNum)
			{
				p = pOri + i;
				break;
			}
		}
	}

	return p;
}

//查找字符
int bytepos(const unsigned char* pSrc, unsigned short nSrc,const char* pSub, unsigned short startPos)
{
    unsigned short sublen  = strlen_t(pSub);
    int _return = -1;
    unsigned short index;
    int  cmp_return;

    if (sublen > nSrc)
    {
        return _return;
    }

    for (index = startPos; index <= (nSrc - sublen); index++)
    {
        cmp_return = memcmp_t(&pSrc[index], pSub, sublen);

        if (cmp_return == 0)
        {
            _return = index;
            return _return;
        }
    }

    return _return;
}

void parse_lat_or_lon_to_double(char *str, double *ret)
{
    char degree[5] = {0};
    char cent[64] = {0};
    char *p = str;
    
    degree[0] = *p;
    p++;
    degree[1] = *p;
    p++;
    degree[2] = *p;
    p++;

    *ret = myatof(degree);

    strcpy_t(cent, p);

    *ret = (*ret) + (myatof(cent) /60);
}

//Toupper
unsigned char Toupper(unsigned char c)
{
	if (islower(c))
	{
		c -= 'a'-'A';
	}
	return c;
}

//double to string
char *f2s(double f, int bit, char *out)
{
	char *pc = NULL;
	char tmp[10] = {0};
	int i;
	int dec;
	double digit;

	pc = out;
	i = 0;
	dec = f;		//整数部分
	digit = f - dec;//小数部分

	while(dec>0)
	{
		tmp[i++] = dec%10 + '0';
		dec /= 10;
	}
	while(i>0)
	{
		*pc++ = tmp[--i];
	}

	*pc++ = '.';

	while(bit--)
	{
		dec = digit*10;
		*pc++ = dec + '0';
		digit = digit*10 - dec;
	}

	return out;
}

//0891683110304405F00411B00156051060649299F80008020113311591231A004100530031003200330034002A004600310030002300420023
unsigned short usc2_ascii(unsigned char *pSrc, unsigned char *pDst, unsigned short nSrcLength)
{
	unsigned short i,c;
	
	for(i=0,c=0; i<nSrcLength; i++)
	{
		if(*pSrc == 0)
		{
			pSrc ++;
			*pDst++ = *pSrc++;
			c ++;
		}
		else
		{
			break;
		}
	}

	return c;
}

unsigned char xor_check(unsigned char *data, unsigned int len)
{
	unsigned char result = data[0];
	unsigned int i = 0;
	
	while(++i < len)
	{
		result = result ^ data[i];
	}

	return result;
}
