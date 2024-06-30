#ifndef __USR_SYS_H__
#define __USR_SYS_H__

#include "usr_gps.h"
#include "usr_dev.h"


/************************************************************************/
#define IP_LEN	 	    15             	//IP�ĳ���
#define APN_LEN		    40             	//APN�ĳ���
#define URL_LEN         80             	//�����ĳ���

#define URT_NUM 			0x03
//uart type set
#define URT_TYPE_NUL 		0x00
#define URT_TYPE_232 		0x01  
#define URT_TYPE_485 		0x02

//uart mask set
#define URT_MASK_NULL 		0x00	// 0
//��ͨ
#define URT_MASK_JT_OIL 	0x01	// 1
//����
#define URT_MASK_CR_OIL 	0x02	// 2
//�õ�
#define URT_MASK_XD_OIL		0x04	// 3
//BT
#define URT_MASK_BT			0x08	// 4


//ͨѶ����
typedef struct _MODEM_COMM_INFO_
{
	unsigned char sCenterIp[IP_LEN+1];
	unsigned char sApn[APN_LEN];
	unsigned char sUser[APN_LEN];
	unsigned char sPasswd[APN_LEN];	
	unsigned short nCenterPort;
	unsigned char nNetType;//���ӷ�ʽ 0 udp 1 tcp
	unsigned char sURL[URL_LEN];//����

	unsigned char sCenterIpBak[IP_LEN+1];
	unsigned short nCenterPortBak;
	unsigned char nNetTypeBak;//�������ӷ�ʽ 0 udp 1 tcp 
	unsigned char sURLBak[URL_LEN-24];//��������

	unsigned char sIMEI[16];//imei

}modem_comm_info;

#define FAKE_IP_LE	    		6        //�ն����кŵĳ���
#define SET_CAR_LICENCES_LEN	16		 //���ƺ��볤��
#define SET_LOGIN_SEL_LEN		40		 //�ն˼�Ȩ�볤��
//�豸ID��
typedef struct _AW_LOGIN_INFO_
{
	unsigned char sFakeIp[FAKE_IP_LE];	//SIM ID
	unsigned short nProvince_ID;		//ʡ��ID
	unsigned short nCity_ID;    		//����ID
	unsigned char sManuCode[6]; 		//������ID
	unsigned char sTerType[21];		//�ն��ͺ�
	unsigned char sTerID[8];    		//�ն�ID
	unsigned char nCarColor;    		//������ɫ
	unsigned char sImeiDevID[8];
	unsigned char sCarLicences[SET_CAR_LICENCES_LEN];	//���ƺ�
	unsigned char sCarVin[SET_CAR_LICENCES_LEN+2];		//����VIN
	unsigned char macT[9];
	unsigned char macK[9];
}login_info;

//ͨ�ò���
//�˴���ӻָ�������Ӱ��Ĳ���
typedef struct _PARAM_INFO_
{
	unsigned char timezone;		//ʱ��
	unsigned char timetype;		//������
	unsigned char canRequest;		//CAN��������
	unsigned char ackSwitch;		//ACK
	unsigned char resv1;			//����
	unsigned char resv2;			//����
	unsigned short can1Baud;  		//CAN������
	unsigned short can2Baud;  		//CAN������
	unsigned int uartMask[URT_NUM];
}param_info;

typedef struct _G_SYS_INFO_
{
	modem_comm_info g_comm;
	login_info      g_login;
	param_info		g_param;
	unsigned char  FwVersion[32]; 	//�汾��
	unsigned int g_chksum;		//У��
}g_sys_info;

typedef struct _G_SN_INFO_
{
	unsigned char nTermSN[20];					 	 //SN��
}g_sn_info;
/************************************************************************/

/************************************************************************/
#define PHONE_NUM		3
#define PHONE_LEN	    15             		//�绰�ĳ��� 
#define PASSWD_LEN      8					//�ն����볤��
//ͨ������
typedef struct _MODEM_PHONE_INFO_
{
	unsigned char nCallCtrl;							//ͨ������
	unsigned char nAutoAnswer; 						//�Զ�����(�Զ�����������)
	unsigned char sTerminalPwd[PASSWD_LEN];			//�ն˵Ķ�������
	unsigned char authorPhone[PHONE_NUM][PHONE_LEN];	//��Ȩ����
}modem_phone_info;

//gps��ز���
typedef struct _GPS_RUN_INFO_
{ 
	unsigned short ageTypeNum;        		//���ϵ��    
	unsigned int nMoveDistance;        	//λ�Ʊ����ľ���   	(��λ����)
	unsigned int nOverSpeed;           	//���ٷ�ֵ
	unsigned int nOverSpeedTime;       	//���ٱ�������ʱ�䷧ֵ (��λ����)
	unsigned int nLowSpeedTime;        	//���ٵĳ���ʱ�䷧ֵ	(��λ����)   
	unsigned int nDriveTimeOut;        	//��ʱ��ʻ�ļ�ʻʱ��  	(��λ����)
	unsigned int nRestTimeOut;        	//��ʱ��ʻ����Ϣʱ��   (��λ����)
	unsigned int nBaseMileage;           //��̻�׼ֵ
	unsigned int nIP2KeepAlive;			//IP2����ʱ��
}gps_run_info; 

//��ز���
typedef struct _GPS_WATCH_INFO_
{
	unsigned short	nWatchInterval; 	//��ؼ��(ACC��)
	unsigned short	nWatchIntervaloff; 	//��ؼ��(ACC��)
	unsigned short	nWatchDistance; 	//��ؾ���
	unsigned char	nWatchType;			// 0 disable, 1 time ,2 distance,3 both
	unsigned char	nRedeem;			//�յ㲹��
	unsigned char	nSleepAble;			//����ʹ��
	unsigned int 	sleepTime;  		//����ʱ��
	unsigned int	nsleepInterval;	//���߻��Ѻ���ʱ��
}gps_watch_info;

typedef struct _G_RUNTIME_INFO_
{
	gps_watch_info   		g_watch; 
	gps_run_info     		g_gpsrun;
	dev_info		 		g_dev; 
	modem_phone_info 		g_phone;
	unsigned int 					g_chksum;
}g_runtime_info;
/************************************************************************/
//������
typedef union _G_FLAG_INFO
{
	struct
	{
		unsigned int MutexFlag 			: 1;
		unsigned int InitFinshFlag	 	: 1;
		unsigned int MainPowFlag	 		: 1;
		unsigned int GpsPowFlag			: 1;
		unsigned int cpinReadyFlag		: 1;
		unsigned int socketPPPFlag		: 1;
		unsigned int otaWaitFlag			: 1;
		unsigned int GpsCmdFlag			: 1;
		unsigned int setCaltimeFlag		: 1;
		unsigned int loginIP2Flag		: 1;
		unsigned int connIP2Flag			: 1;
		unsigned int openIP2Flag			: 1;
		unsigned int rfidbuf				: 1;
		unsigned int RfidLink			: 1;
		unsigned int bit14				: 1;
		unsigned int bit15				: 1;
		unsigned int bit16				: 1;
		unsigned int bit17				: 1;
		unsigned int bit18				: 1;
		unsigned int bit19				: 1;
		unsigned int bit20				: 1;
		unsigned int bit21				: 1;
		unsigned int bit22				: 1;
		unsigned int bit23				: 1;
		unsigned int bit24				: 1;
		unsigned int bit25				: 1;
		unsigned int bit26				: 1;
		unsigned int bit27				: 1;
		unsigned int bit28				: 1;
		unsigned int bit29				: 1;
		unsigned int bit30				: 1;
		unsigned int bit31				: 1;
	}bit;
	unsigned int value;
}g_flag_info;

typedef enum
{
	INIT_NULL = 0,
	INIT_OK,
}INIT_STATUS;

typedef union _G_DEVID_INFO
{
	unsigned char buf[8];
	long long value;
}g_devid_info;

/************************************************************************/
//��̲���
typedef struct _G_MILE_INFO_
{
	unsigned int nMileAge;             	//���ͳ�Ƶ�λm	
}g_mile_info;
/************************************************************************/

typedef enum AuthorPhoneListTag
{
    AUTHORPHONE_NULL = 0,
	AUTHORPHONE_CMP_SUCCESS,
	AUTHORPHONE_CMP_FAIL,
}AuthorPhoneList;

typedef enum CallPhoneListTag
{
    CALLPHONE_NULL = 0,
	CALLPHONE_FORBID_DIAL,		//��ֹ����ͨ��
	CALLPHONE_FORBID_ATA,		//��ֹ�����绰
	CALLPHONE_FORBID_DIAL_ATA,	//��ֹ����ͽ���
	CALLPHONE_ALLOW_DIAL_ATA,	//������ͽ���
}CallPhoneList;

extern g_sys_info  g_sys;
extern g_sn_info  g_sn;
extern g_runtime_info g_runtime;
extern g_mile_info g_mile;
extern g_flag_info g_flag;

void sys_Init(void);
void sys_check_rst(void);
unsigned short sys_param_crc16(unsigned char d[], unsigned int len);
void sys_saveSys(void);
void sys_readSys(void);
void sys_saveRuning(void);
void sys_readRuning(void);
void sys_saveSN(void);
void sys_readSN(void);
void sys_clear_alarm(void);
void sys_SoftReset(void);
void sys_reset_offline(void);
void sys_ClearBlindIndex(void);
void sys_clear_run(void);
void sys_reset_day(void);
void sys_SetSysRTC(TIME *rtc);
TIME *sys_GetSysRTC(void);

unsigned char *sys_GetIp(void);
void sys_SetIp(unsigned char* buf);
void sys_ClrIp(void);
unsigned short sys_GetPort(void);
void sys_SetPort(unsigned short port);
unsigned char sys_GetLinkType(void);
void sys_SetLinkType(unsigned char type);
unsigned char *sys_GetUrl(void);
void sys_SetUrl(unsigned char *buf);
void sys_ClearUrl(void);

unsigned char *sys_GetIpBak(void);
void sys_SetIpBak(unsigned char* buf);
void sys_ClrIpBak(void);
unsigned short sys_GetPortBak(void);
void sys_SetPortBak(unsigned short port);
unsigned char sys_GetLinkTypeBak(void);
void sys_SetLinkTypeBak(unsigned char type);
unsigned char *sys_GetUrlBak(void);
void sys_SetUrlBak(unsigned char *buf);
void sys_ClearUrlBak(void);



void sys_SetId(unsigned char* buf);
unsigned char *sys_GetId1(void);
void sys_GetId2(unsigned char *buf);
void sys_SetCarVin(unsigned char* buf);
unsigned char* sys_GetCarVin(void);
void sys_SetTermId(unsigned char* buf);
unsigned char *sys_GetTermId(void);
void sys_SetProvinceId(unsigned short data);
unsigned short sys_GetProvinceId(void);
void sys_SetCityId(unsigned short data);
unsigned short sys_GetCityId(void);
void sys_SetManuId(unsigned char* buf);
unsigned char* sys_GetManuId(void);
void sys_SetTermType(unsigned char* buf);
unsigned char* sys_GetTermType(void);
void sys_SetCarColor(unsigned char data);
unsigned char sys_GetCarColor(void);
void sys_SetCarLicences(unsigned char* buf);
unsigned char* sys_GetCarLicences(void);

void sys_SetImeiDevID(unsigned char* buf);

unsigned char *sys_GetTermSN(void);
void sys_SetTermSN(unsigned char* buf);

unsigned char *sys_GetApn(void);
void sys_SetApn(unsigned char *buf);
unsigned char *sys_GetApnUser(void);
void sys_SetApnUser(unsigned char *buf);
unsigned char *sys_GetApnPasswd(void);
void sys_SetApnPasswd(unsigned char *buf);
void sys_SetTerminalPwd(unsigned char *buf);
unsigned char *sys_GetTerminalPwd(void);
unsigned char sys_CheckTerminalPwd(unsigned char *buf);

void sys_SetUartFunc(unsigned char no,unsigned int data);
unsigned int sys_GetUartFunc(unsigned char no);

void sys_SetAckSwitch(unsigned char ack);
unsigned char sys_GetAckSwitch(void);
void sys_SetCanRequestAble(unsigned char ctl);
unsigned char sys_GetCanRequestAble(void);
void sys_SetAuthorPhone(unsigned char cn, unsigned char *buf);
unsigned char *sys_GetAuthorPhone(unsigned char cn);
void sys_ClrAuthorPhone(void);
unsigned char sys_CheckAuthorPhone(unsigned char *buf);
void sys_SetAutoAnswerTime(unsigned char data);
unsigned char sys_GetAutoAnswerTime(void);
void sys_SetCallCtrl(unsigned char data);
unsigned char sys_GetCallCtrl(void);

void sys_SetageNum(unsigned char data);
unsigned char sys_GetageNum(void);
void sys_SetMileAge(unsigned int mileage);
void sys_AddMileAge(unsigned int mileage);
unsigned int sys_GetMileAge(void);
void sys_set_debugSwitch(char on);
unsigned char sys_get_debugSwitch(void);
void sys_SetTimeZone(unsigned char zone);
unsigned char sys_GetTimeZone(void);
void sys_SetTimeType(unsigned char type);
unsigned char sys_GetTimeType(void);

void sys_SetAccStateOffInterval(unsigned short data);
unsigned short sys_GetAccStateOffInterval(void);
void sys_SetAccStateOnInterval(unsigned short data);
unsigned short sys_GetAccStateOnInterval(void);
void sys_SetDistanceInterval(unsigned short data);
unsigned short sys_GetDistanceInterval(void);
void sys_SetWatchType(unsigned char type);
unsigned char sys_GetWatchType(void);
void sys_SetRedeem(unsigned char data);
unsigned char sys_GetRedeem(void);


void sys_SetOverSpeed(unsigned char speed);
unsigned char sys_GetOverSpeed(void);
void sys_SetOverSpeedTime(unsigned short time);
unsigned short sys_GetOverSpeedTime(void);
void sys_SettLowSpeedTime(unsigned short time);
unsigned short sys_GetLowSpeedTime(void);
void sys_SetDriveTimeOut(unsigned short time);
unsigned short sys_GetDriveTimeOut(void);
void sys_SetRestTimeOut(unsigned short time);
unsigned short sys_GetRestTimeOut(void);


void sys_SetBaseMileage(unsigned int mile);
unsigned int sys_GetBaseMileage(void);
void sys_SetMoveDistance(unsigned short distance);
unsigned short sys_GetMoveDistance(void);
void sys_SetIP2KeepAlive(unsigned int keepalive);
unsigned int sys_GetIP2KeepAlive(void);

void sys_SetCan1Baud(unsigned short baud);
unsigned short sys_GetCan1Baud(void);
void sys_SetCan2Baud(unsigned short baud);
unsigned short sys_GetCan2Baud(void);
/**********************************************sleep***********************************************************************/
void sys_SetSleepAble(unsigned char type);
unsigned char sys_GetSleepAble(void);
void sys_SetSleepWakeUpInterval(unsigned short data);
unsigned short sys_GetSleepWakeUpInterval(void);
void sys_SetSleepTimeWakeUp(unsigned int data);
/**********************************************sleep***********************************************************************/
/***********************************************************************************/
void sys_SetOilVolume(unsigned char id, unsigned short data);
unsigned short sys_GetOilVolume(unsigned char id);
void sys_SetOilToSteal(unsigned char data);
unsigned char sys_GetOilToSteal(void);
void sys_SetOilToLow(unsigned char data);
unsigned char sys_GetOilToLow(void);
/***********************************************************************************/

GPSANT_STATUS sys_GetGpsState(void);
gpsinfo* sys_GetLastGpsData(void);
void sys_SetLastGpsData(gpsinfo *data);
void sys_ClearRuning(void);
void sys_SetDefaultParam(void);
void sys_Restart(void);
void sys_ClearFlash(bool bfactory);
void sys_ResetFacitory(void);
void sys_clear_runFlag(void);
void sys_SetUpdateFlag(unsigned char flag);
unsigned char sys_GetUpdateFlag(void);
void sys_ReadUpdate_Flag(void);
void sys_backupApp_to_ExtFlash(void);
unsigned char sys_CheckbackupApp(void);
void sys_auto_backupApp(void);
void sys_noinit_clear(void);
void sys_noinit_state(void);
void sys_SetOTASerialNo(unsigned short no);
unsigned short sys_GetOTASerialNo(void);

void sys_state_gpsloc(unsigned int on);
void sys_state_obd(unsigned int on);
void sys_state_newCar(unsigned int x);
void sys_state_newAlarm(unsigned int x);
void sys_state_newSysState(unsigned int x);
void sys_state_offCar(unsigned int x);
void sys_state_offAlarm(unsigned int x);
void sys_state_offSysState(unsigned int x);
unsigned int sys_state_checkCar(unsigned int x);
unsigned int sys_state_checkAlarm(unsigned int x);
unsigned int sys_state_checkSysState(unsigned int x);
unsigned int sys_state_get_acc(void);
unsigned int sys_state_get_gpsloc(void);
unsigned int sys_state_get_car(void);
unsigned int sys_state_get_sys(void);
unsigned int sys_state_get_alarm(void);
unsigned int sys_state_get_power(void);
unsigned int sys_state_get_gpslastloc(void);
void sys_clear_lastGpsA(void);
void sys_state_lastGpsA(void);

void sys_state_newCar9(unsigned char x);
void sys_state_offCar9(unsigned char x);
unsigned char sys_state_checkCar9(unsigned char x);
unsigned char sys_state_getCar9(void);
void sys_state_acc9(unsigned int on);
void sys_state_sos9(unsigned int on);
void sys_state_di19(unsigned int on);
void sys_state_di29(unsigned int on);
void sys_state_di39(unsigned int on);
void sys_state_do19(unsigned int on);
void sys_state_do29(unsigned int on);
void sys_state_do39(unsigned int on);
#endif

