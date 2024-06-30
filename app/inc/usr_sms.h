#ifndef __USR_SMS_H__
#define __USR_SMS_H__

//≥ı º√‹¬Î
#define SMS_INITIAL_PASSWORD	"1234"
//≥¨º∂√‹¬Î
#define SMS_SUPER_PASSWORD		"7777"

#define MAX_PHONE_NUM    		20
#define MAX_UCS2_SMS_LENGTH		70
#define MAX_TEXT_SMS_LENGTH  	160
#define MAX_PDU_SMS_LENGTH  	180

typedef enum SMSStatusListTag
{
    SMS_NULL = 0,
	SMS_DISCONNECT_TCPIP,
	SMS_RESTART,
	SMS_FACTORY,
}SMSStatusList;

typedef enum SMSChsetTag
{
    SMS_CHSET_GSM  = 0,
    SMS_CHSET_UCS2 = 1,
    SMS_CHSET_8BIT =  2,
    SMS_CHSET_INVALID = 255
}SMSChset;

typedef struct _SMSTextMsgTag
{
	unsigned char oa;
    unsigned char phone_num[MAX_PHONE_NUM];
    unsigned char data[MAX_TEXT_SMS_LENGTH*2];
    unsigned short data_len;
	SMSChset dcs;
}SMSTextMsg;

typedef struct SMSCmdTag
{
	unsigned char  *cmd;
	int (*cmd_handler)(unsigned char *str, unsigned char *out);
}SMS_T;

void sms_noinit_clear(void);
void sms_get_phone(void);
void sms_MsgHandle(SMSTextMsg *pDeliverTextInfo);
void sms_DelAllSms(void);
void sms_ReadSms(unsigned short index);
void sms_RecvSms(unsigned char *pSmsText, unsigned char index);
void sms_TimerCheck(void);
void sms_CheckSms(void);
void sms_SendSms(void);
unsigned char sms_MsgSend(SMSTextMsg *pSubmitTextInfo);
unsigned char sms_LongMsgSend(SMSTextMsg *pSubmitTextInfo, unsigned char totalSMS, unsigned char numSMS);
unsigned char sms_SendAtCmd(unsigned char len,unsigned char *data);
unsigned char sms_Delete(unsigned short index);
unsigned char sms_CheckHaveAutor(void);
void sms_SendAlarmAutor(SMSTextMsg *pSubmitTextInfo);
void app_otaResultReport(unsigned char flag);
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
unsigned char ValueToChar(unsigned char ch);
unsigned char ValueOfChar(unsigned char ch);
unsigned int Dchar2Int(unsigned char* s);
void strReverse(unsigned char *src,unsigned char *dst,unsigned int srclen);
void strReverseF(unsigned char *src, unsigned char *dst,unsigned int srclen);
void Str2pdu(unsigned char *str,unsigned int strlen,unsigned char *pdu);
void Pdu2Str(unsigned char *pdu,unsigned char pdulen,unsigned char *str);
unsigned int ASCII2GSM7bit(const unsigned char *pinSms,unsigned int srclen,unsigned char *poutSms);
unsigned int GSM7bit2ASCII(const unsigned char *pinSms, unsigned int srclen,unsigned char *poutSms);

#endif
