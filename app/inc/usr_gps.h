#ifndef __USR_GPS_H__
#define __USR_GPS_H__

#define MAX_GPS_UART_RCV_LEN 	1024
typedef struct
{
	unsigned short usMsgLen;
	unsigned char  ucMsgRcvBuf[MAX_GPS_UART_RCV_LEN];
} xGpsParameters;

typedef enum
{
	MSG_GPS_RECV_REQ = 0,
	MSG_GPS_TIME_REQ,
	MSG_GPS_POWOFF_REQ,
	MSG_GPS_POWON_REQ,

}GPSTASK_EventList;

typedef enum
{
	GPS_POWON = 0,
	GPS_POWOFF,
}GPSPOW_STATUS;

typedef enum
{
	CAR_STATIC = 0,
	CAR_MOVE,
}CAR_STATUS;

typedef enum
{
	GPS_ANTV = 0,
	GPS_ANTA,
	GPS_ANTSHORT,
	GPS_ANTOPEN,
	GPS_ANTFAULT,
}GPSANT_STATUS;

#define OA_NMEA_MAX_LEN 100
#define OA_NMEA_STR_LEN 16
#define OA_NMEA_MIN_LEN 8

/*有效定位卫星个数*/
#define GPS_SATELLITE_COUNT        4
/*有效定位精度*/
#define GPS_SATELLITE_PRECISION    10

#define XOR_FAILED     0
#define XOR_SUCCESS    1

typedef struct _TIME
{
    unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}TIME;

typedef struct _GPS_INFO_
{
    char latitude_str[OA_NMEA_STR_LEN];
    char longitude_str[OA_NMEA_STR_LEN];
    char latitude_flag;
    char longitude_flag;
    char speed_str[OA_NMEA_MIN_LEN];
	char head_str[OA_NMEA_MIN_LEN];
    char sat_num;
    char hdop;
	char status;
    TIME date;
}gpsinfo;

typedef struct _Point
{
	double x;		// x经度dbLon，以度为单位
	double y;		// y纬度dbLat，以度为单位
}Point;

extern xQueueHandle xGpsQueueId;
extern xGpsParameters xGpsRecvQueue;
extern gpsinfo g_gpsinfo;
extern gpsinfo g_gpsLastinfo;

void gps_Init(void);
void gps_uartInit(void);
void gps_powerOn(void);
void gps_powerOff(void);
void agps_sendCmd(unsigned char *buf,unsigned short lem);
void gps_clearBuf(void);
bool gps_update_location_condition(void);
void gps_date_utc_to_rtc(char timezone, TIME *currTime, TIME *resultTime);
gpsinfo  *gps_GetGpsInfo(void);
unsigned int gps_GetGpsSpeed(void);
unsigned char *gps_GetGprmc(void);
void gps_GetGpsHead(unsigned char *data);
void gps_GetGpsTime(TIME *data);
void gps_watch_proc(void);
void gps_AlarmHandle(void);
unsigned char gps_GetGpsStarNum(void);
unsigned short gps_GetGpsAltitude(void);
void gps_recv(unsigned char *data, unsigned short len);

void vGpsTaskQueueCreat(void);
void vGpsTask(void *pvParameters);

#endif

