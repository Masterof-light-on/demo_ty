#ifndef __USR_SYS_H__
#define __USR_SYS_H__

#include "usr_gps.h"
#include "usr_dev.h"


/************************************************************************/
#define IP_LEN	 	    15             	//IP的长度
#define APN_LEN		    40             	//APN的长度
#define URL_LEN         80             	//域名的长度

#define URT_NUM 			0x03
//uart type set
#define URT_TYPE_NUL 		0x00
#define URT_TYPE_232 		0x01  
#define URT_TYPE_485 		0x02

//uart mask set
#define URT_MASK_NULL 		0x00	// 0
//久通
#define URT_MASK_JT_OIL 	0x01	// 1
//长润
#define URT_MASK_CR_OIL 	0x02	// 2
//忻德
#define URT_MASK_XD_OIL		0x04	// 3
//BT
#define URT_MASK_BT			0x08	// 4


//通讯参数
typedef struct _MODEM_COMM_INFO_
{
	unsigned char sCenterIp[IP_LEN+1];
	unsigned char sApn[APN_LEN];
	unsigned char sUser[APN_LEN];
	unsigned char sPasswd[APN_LEN];	
	unsigned short nCenterPort;
	unsigned char nNetType;//连接方式 0 udp 1 tcp
	unsigned char sURL[URL_LEN];//域名

	unsigned char sCenterIpBak[IP_LEN+1];
	unsigned short nCenterPortBak;
	unsigned char nNetTypeBak;//备份连接方式 0 udp 1 tcp 
	unsigned char sURLBak[URL_LEN-24];//备份域名

	unsigned char sIMEI[16];//imei

}modem_comm_info;

#define FAKE_IP_LE	    		6        //终端序列号的长度
#define SET_CAR_LICENCES_LEN	16		 //车牌号码长度
#define SET_LOGIN_SEL_LEN		40		 //终端鉴权码长度
//设备ID号
typedef struct _AW_LOGIN_INFO_
{
	unsigned char sFakeIp[FAKE_IP_LE];	//SIM ID
	unsigned short nProvince_ID;		//省域ID
	unsigned short nCity_ID;    		//市域ID
	unsigned char sManuCode[6]; 		//制造商ID
	unsigned char sTerType[21];		//终端型号
	unsigned char sTerID[8];    		//终端ID
	unsigned char nCarColor;    		//车牌颜色
	unsigned char sImeiDevID[8];
	unsigned char sCarLicences[SET_CAR_LICENCES_LEN];	//车牌号
	unsigned char sCarVin[SET_CAR_LICENCES_LEN+2];		//车辆VIN
	unsigned char macT[9];
	unsigned char macK[9];
}login_info;

//通用参数
//此处添加恢复出厂不影响的参数
typedef struct _PARAM_INFO_
{
	unsigned char timezone;		//时区
	unsigned char timetype;		//东西经
	unsigned char canRequest;		//CAN数据请求
	unsigned char ackSwitch;		//ACK
	unsigned char resv1;			//保留
	unsigned char resv2;			//保留
	unsigned short can1Baud;  		//CAN波特率
	unsigned short can2Baud;  		//CAN波特率
	unsigned int uartMask[URT_NUM];
}param_info;

typedef struct _G_SYS_INFO_
{
	modem_comm_info g_comm;
	login_info      g_login;
	param_info		g_param;
	unsigned char  FwVersion[32]; 	//版本号
	unsigned int g_chksum;		//校验
}g_sys_info;

typedef struct _G_SN_INFO_
{
	unsigned char nTermSN[20];					 	 //SN号
}g_sn_info;
/************************************************************************/

/************************************************************************/
#define PHONE_NUM		3
#define PHONE_LEN	    15             		//电话的长度 
#define PASSWD_LEN      8					//终端密码长度
//通话参数
typedef struct _MODEM_PHONE_INFO_
{
	unsigned char nCallCtrl;							//通话限制
	unsigned char nAutoAnswer; 						//自动接听(自动接听的秒数)
	unsigned char sTerminalPwd[PASSWD_LEN];			//终端的短信密码
	unsigned char authorPhone[PHONE_NUM][PHONE_LEN];	//授权号码
}modem_phone_info;

//gps相关参数
typedef struct _GPS_RUN_INFO_
{ 
	unsigned short ageTypeNum;        		//里程系数    
	unsigned int nMoveDistance;        	//位移报警的距离   	(单位是米)
	unsigned int nOverSpeed;           	//超速阀值
	unsigned int nOverSpeedTime;       	//超速报警持续时间阀值 (单位是秒)
	unsigned int nLowSpeedTime;        	//怠速的持续时间阀值	(单位是秒)   
	unsigned int nDriveTimeOut;        	//超时驾驶的驾驶时间  	(单位是秒)
	unsigned int nRestTimeOut;        	//超时驾驶的休息时间   (单位是秒)
	unsigned int nBaseMileage;           //里程基准值
	unsigned int nIP2KeepAlive;			//IP2在线时间
}gps_run_info; 

//监控参数
typedef struct _GPS_WATCH_INFO_
{
	unsigned short	nWatchInterval; 	//监控间隔(ACC开)
	unsigned short	nWatchIntervaloff; 	//监控间隔(ACC关)
	unsigned short	nWatchDistance; 	//监控距离
	unsigned char	nWatchType;			// 0 disable, 1 time ,2 distance,3 both
	unsigned char	nRedeem;			//拐点补偿
	unsigned char	nSleepAble;			//休眠使能
	unsigned int 	sleepTime;  		//休眠时间
	unsigned int	nsleepInterval;	//休眠唤醒后工作时长
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
//不保存
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
//里程参数
typedef struct _G_MILE_INFO_
{
	unsigned int nMileAge;             	//里程统计单位m	
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
	CALLPHONE_FORBID_DIAL,		//禁止拨打通话
	CALLPHONE_FORBID_ATA,		//禁止接听电话
	CALLPHONE_FORBID_DIAL_ATA,	//禁止拨打和接听
	CALLPHONE_ALLOW_DIAL_ATA,	//允许拨打和接听
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

