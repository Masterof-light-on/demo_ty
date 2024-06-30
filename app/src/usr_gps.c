#include "includes.h"

/* 队列句柄 */
xQueueHandle xGpsQueueId;
xGpsParameters xGpsRecvQueue;

gpsinfo g_gpsinfo;
gpsinfo g_gpsLastinfo;

#define INPUT_GPS_LEN   120
#define MAX_GPS_TYPE	6
static unsigned char szcGpsBuf[MAX_GPS_TYPE][INPUT_GPS_LEN] = {0};
static unsigned char sGPRMC[INPUT_GPS_LEN] = {0};

static unsigned char g_gps_star_num = 0;
static unsigned short gpsErrorTime  = 0;
static unsigned short g_gps_altitude= 0;

const char *sGpsType[] = 
{
	"$GPGGA",
	"$GPRMC",
	"$BDGGA",
	"$BDRMC",
	"$GNGGA",
	"$GNRMC"
};

//gps初始化
void gps_Init(void)
{
	gps_powerOn();
}
//gps串口初始化
void gps_uartInit(void)
{
	uart1_init(115200);
}
//gps发送
void gps_sendCmd(unsigned char *cmd)
{
	print1(cmd, strlen_t((const char *)(cmd)));
}
void agps_sendCmd(unsigned char *buf,unsigned short lem)
{
	print1(buf, lem);
}
//gps上电
void gps_powerOn(void)
{
	g_flag.bit.GpsPowFlag = GPS_POWON;
	gps_uartInit();
	gpio_Gps_powerCtrl(1);
}
//gps下电
void gps_powerOff(void)
{
	g_flag.bit.GpsPowFlag = GPS_POWOFF;
	g_flag.bit.GpsCmdFlag = 0;
	//GPS串口输出低
	gpio_dir(PA2, 1);
	gpio_set(PA2, 0);
	gpio_dir(PA3, 1);
	gpio_set(PA3, 0);
	gpio_Gps_powerCtrl(0);
}
//gps模块重启
void gps_reset(void)
{
	OS_SendMessage(xGpsQueueId, MSG_GPS_POWOFF_REQ, 0, 0);
}
//中科微模块
void gps_zkw_proc(void)
{
	if(g_flag.bit.GpsPowFlag == GPS_POWON)
	{
		if(g_flag.bit.GpsCmdFlag == 0)
		{
			g_flag.bit.GpsCmdFlag = 1;
			gps_sendCmd("$PCAS03,1,0,0,0,1,0,0,1,0,0,,,0,0,,,,0*33\r\n");
		}
	}
}

//xor_check gps data
unsigned char gps_nmea_xor_check(unsigned char *nmea_line, int len_nmea)
{
	int ret = XOR_FAILED;
	int chcount;
	unsigned char xor;
	unsigned char expect_xor;
	unsigned char c;

	xor = 0;
	chcount = 1;
	while ((chcount < len_nmea) && (nmea_line[chcount] != '*')){
		xor ^= (unsigned char )nmea_line[chcount++];
	}

	if ( nmea_line[chcount] != '*' ) return ret;

	c = Toupper(nmea_line[chcount + 1]);
	if(c >= 'A' && c <= 'F')
		c = c - 'A' + 10;
	else
		c = c - '0';
	expect_xor = c << 4;

	c = Toupper(nmea_line[chcount + 2]);
	if(c >= 'A' && c <= 'F')
		c = c - 'A' + 10;
	else
		c = c - '0';
	expect_xor |= c;

	if(expect_xor == xor)
		ret = XOR_SUCCESS;

	return ret;
}
bool gps_nmea_skip_token(unsigned char **data, unsigned short *len, char token)
{
	unsigned short i;

	for(i = 0; i < (*len); i++)
	{
		if((*data)[i] == token)
		{
			break;
		}		
	}

	(*data) += (i + 1);
	(*len)  -= (i + 1);

	return TRUE;
}
bool gps_nmea_time_token(unsigned char **data, unsigned short *len, char token, TIME *time)
{
	unsigned short i;
	char timestring[15] = {0};
	char sec[2 + 1] = {0};
	char min[2 + 1] = {0};
	char hour[2 + 1] = {0};
	char* t_point=NULL;

	for(i = 0; i < (*len); i++)
	{
		if((*data)[i] == token)
		{
			break;
		}
	}
	if(i == 10 || i == 9)//真实数据格式：$GPRMC,042410.000,A,3908.9373,N,11616.0758,E,9.94,180.0,030713,,,A*51
	{
		t_point = strchr_t((char*)(*data), '.');
		if(!t_point) 
			return FALSE;
	}
	else if(i == 6)//模拟数据格式：$GPRMC,041217,A,2236.9452,N,11351.4054,E,30.82,162.68,030713,,,A*48
	{
		;
	}
	else
	{
		return FALSE;
	}

	memcpy_t(timestring, (*data), i);
	timestring[i] = 0;

	if(Is_Digit(timestring,6) == 0)
	{
		LOG("[gps] time error\r\n");
		return FALSE;	
	}

	sec[1] = timestring[5];
	sec[0] = timestring[4];

	min[1] = timestring[3];
	min[0] = timestring[2];

	hour[1] = timestring[1];
	hour[0] = timestring[0];

	time->hour = myatoi(hour);
	time->minute = myatoi(min);
	time->second = myatoi(sec);

	(*data) += (i + 1);
	(*len)  -= (i + 1);

	return TRUE;
}
bool gps_nmea_char_token(unsigned char **data, unsigned short *len, char token, char *v)
{
	unsigned short i;

	for(i = 0; i < (*len); i++)
	{
		if((*data)[i] == token)
		{
			break;
		}		
	}

	*v = **data;
	(*data) += (i + 1);
	(*len)  -= (i + 1);

	return TRUE;
}
bool gps_nmea_string_token(unsigned char **data, unsigned short *len, char token, char *v, unsigned short max_len)
{
	unsigned short i;
	bool ret = TRUE;

	for(i = 0; i < (*len); i++)
	{
		if((*data)[i] == token)
		{
			break;
		}

		if(i == (max_len - 1))
		{
			ret = FALSE;
			break;
		}
	}

	memcpy_t(v, (*data), i);
	v[i] = 0;

	(*data) += (i + 1);
	(*len)  -= (i + 1);
	
	return ret;
}
bool gps_nmea_date_token(unsigned char **data, unsigned short *len, char token, TIME *date)
{
	unsigned short i;
	char datestring[15] = {0};
	char year[2 + 1] = {0};
	char month[2 + 1] = {0};
	char day[2 + 1] = {0};

	for(i = 0; i < (*len); i++)
	{
		if((*data)[i] == token)
		{
			break;
		}
	}
	if(i != 6)
	{
		return FALSE;
	}

	memcpy_t(datestring, (*data), i);
	datestring[i] = 0;

	if(Is_Digit(datestring,6) == 0)
	{
		LOG("[gps] date error\r\n");
		return FALSE;	
	}

	day[0] = datestring[0];
	day[1] = datestring[1];

	month[0] = datestring[2];
	month[1] = datestring[3];

	year[0] = datestring[4];
	year[1] = datestring[5];

	date->year = myatoi(year);
	date->month = myatoi(month);
	date->day = myatoi(day);

	(*data) += (i + 1);
	(*len)  -= (i + 1);

	return TRUE;
}
/************************************************************************/
//GPS时间转换，涉及到时区的问题
unsigned int rtt_mktime(TIME time, long timezone /*= 8*/)
{
    unsigned int res;
        
    // 1..12 -> 11,12,1..10, Puts Feb last since it has leap day
    if(time.month <= 2)
	{
        time.month += 10;
        time.year  -= 1;
    }
    else
    {
        time.month -= 2;
    }
        
    /**//*
		// Calculate how much days from 1970 to today
		res = 59;                            //31@0001-January and 28@0001-February
		res += (time.year - 1) * 365;        //at least 365 days one year
		res += time.year / 4 - time.year / 100 + time.year / 400;    //leap years from 0001
		res += 367 * time.month / 12 - 30;    //days from March(0.5/7<=slope<0.5/5)
		res += time.day - 1;                //days
		res -= 719162;                        //days from 0001-1-1 to 1970-1-1
		// Calculate how much seconds
		res = ( (res * 24 + time.hour) * 60 + time.minute) * 60 + time.second;
    */
    /**////*
    res = (unsigned int)(time.year/4 - time.year/100 + time.year/400) +
                367*time.month/12 + time.day + time.year*365 - 719499;
    res = ((res*24 + time.hour)        // now have hours
               *60 + time.minute)      // now have minutes
               *60 + time.second;      // finally seconds
    //*/
        
    res -= timezone * 60 * 60;

    return res;
}

int rtt_isleap(unsigned short year)
{
    return ((year%4==0)&&(year%100!=0)||(year%400==0));
}

void rtt_localtime(unsigned int res, TIME *time, long timezone /*= 8*/)
{
    const int monthLengths[2][13] = {
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
    };
    const int yearLengths[2] = { 365, 366 };
    int year     = 0;
    int month    = 0;
    int minMonth = 0;
    int maxMonth = 0;
    
    int days   = 0;
    int clock  = 0;
    int isLeap = 0;
    
    res += timezone*60*60;

    days  = res / 86400;
    clock = res % 86400;
    if(clock < 0)
    {
        clock += 86400;
        days  -= 1;
    }
    
    /**/////////////////////////////////////////////////////////////////////////////
    // Calcaulate year, 11323=0x3A4FC880/86400; 13879=0x47798280/86400
    /**////////////////////////////////////////////////////////////////////////////    
    if(days >= 0)
    {
        year = days/366;
        days -= year*365 + (year+1)/4 - (year+69)/100 + (year+369)/400;
        
        for(time->year = year + 1970; ; time->year++)
        {
            isLeap = rtt_isleap(time->year);
            if(days < yearLengths[isLeap])
            {
				break;
            }
            days -= yearLengths[isLeap];
        }
    }
    else
    {
        year = days/366;
        days -= year*365 + (year-2)/4 - (year-30)/100 + (year-30)/400;
        
        for(time->year = year + 1970 - 1; ; time->year--)
        {
            isLeap = rtt_isleap(time->year);
            days += yearLengths[isLeap];
            
            if(days >= 0)
            {
                break;
            }
        }
    }    
    
    /**///////////////////////////////////////////////////////////////////////////
    // compute month and day, use the half search save time
    /**////////////////////////////////////////////////////////////////////////////
    minMonth = 0;
    maxMonth = 12;
    for(month = 5; month < 12 && month > 0; month = (minMonth + maxMonth) / 2)
    {
        // days between monthLengths[month]<=days<monthLengths[month+1]
        if(days < monthLengths[isLeap][month])				//too big
        {
            maxMonth = month;
        }
        else if(days >= monthLengths[isLeap][month + 1])    //too small
        {
            minMonth = month;
        }
        else    //so it is
        {
            break;
        }
    }
    days -= monthLengths[isLeap][month];
    time->month = month + 1;
    
    time->day = days + 1;
    
    
    /**///////////////////////////////////////////////////////////////////////////
    // Calcaulate hour minute and second
    /**///////////////////////////////////////////////////////////////////////////
    time->hour = clock / 3600;        //3600s one hour
    clock = clock % 3600;
    time->minute = clock / 60;        //60s one minute
    time->second = clock % 60;        //ms
}
/************************************************************************/
//时区转换
void gps_date_utc_to_rtc(char timezone, TIME *currTime, TIME *resultTime)
{   
	TIME utc_time; 
	unsigned int time_second=0;

	utc_time.year   = 2000+currTime->year;
	utc_time.month  = currTime->month;
	utc_time.day    = currTime->day;
	utc_time.hour   = currTime->hour;
	utc_time.minute = currTime->minute;
	utc_time.second = currTime->second;
        
    time_second = rtt_mktime(utc_time, 0);//utc时间， 0 时区
    
    if(sys_GetTimeType() == '-')
    {
        rtt_localtime(time_second, resultTime, -timezone);
    }
    else //if(Sys_GetTimeType() == '+')
    {
		rtt_localtime(time_second, resultTime, timezone);
    }
}

//速度过滤
void gps_filter_speed(void)
{
	unsigned short head=0;
	unsigned int speed=0;

	head = (unsigned short)(myatof(g_gpsinfo.head_str));//方向判断
	if(head > 360)
	{
		strcpy_t(g_gpsinfo.head_str, "00.00");
	}

	speed = (unsigned int)(myatof(g_gpsinfo.speed_str)*1.852f);//km/h
	if((speed <= 5)||(speed > 180))
	{
		strcpy_t(g_gpsinfo.speed_str, "00.00");
		return;
	}
}

//实时更新位置的条件
bool gps_update_location_condition(void)
{
	//掉电报警
	//ACC ON
	if(sys_state_get_power() || \
	   sys_state_get_acc())
	{
		return TRUE;
	}

	return FALSE;
}

//$GPRMC
void gps_nmea_parse_gprmc(unsigned char *data, unsigned short len)
{
//$GPRMC,013123.000,A,2235.5348,N,11352.2562,E,0.79,23.11,030409,,,A*56
	unsigned char *p=data;
	unsigned short l=len;
	char latitude[OA_NMEA_STR_LEN] = {0};
	char longitude[OA_NMEA_STR_LEN]= {0};
	char latitude_flag  = 0;
	char longitude_flag = 0;
	TIME t_date = {0};
    gpsinfo tmp_gpsinfo;
    static unsigned char sFilterSpeedTime=0;
    static unsigned char sFilterSpeedFlag=0;
	static unsigned short gpsNolocalTime =0;

	
	//报文校验
	if(gps_nmea_xor_check(p,len) == XOR_FAILED)
	{
		return;
	}
	//GPRMC
	memset_t(sGPRMC, 0, sizeof(sGPRMC));
	strcpy_t((char *)sGPRMC, (char *)p);
	//跳过头帧
	p += 7;
	l -= 7;
	
	memset_t(tmp_gpsinfo.latitude_str, 0x30, OA_NMEA_STR_LEN);
	memset_t(tmp_gpsinfo.longitude_str, 0x30, OA_NMEA_STR_LEN);    
	memset_t(tmp_gpsinfo.speed_str, 0x30, OA_NMEA_MIN_LEN);
	memset_t(tmp_gpsinfo.head_str, 0x30, OA_NMEA_MIN_LEN);
    gpsErrorTime = 0;//收到数据清零
	
	gps_nmea_time_token(&p, &l, ',', &t_date);//UTC time					
	gps_nmea_char_token(&p, &l, ',', &tmp_gpsinfo.status);//status
	gps_nmea_string_token(&p, &l, ',', latitude, OA_NMEA_STR_LEN);//latitude
	gps_nmea_char_token(&p, &l, ',', &latitude_flag);//latitude indicate
	gps_nmea_string_token(&p, &l, ',', longitude, OA_NMEA_STR_LEN);//longitude
	gps_nmea_char_token(&p, &l, ',', &longitude_flag);//longitude indicate
	gps_nmea_string_token(&p, &l, ',', tmp_gpsinfo.speed_str, OA_NMEA_MIN_LEN);//speed
	gps_nmea_string_token(&p, &l, ',', tmp_gpsinfo.head_str, OA_NMEA_MIN_LEN);//head
	gps_nmea_date_token(&p, &l, ',', &t_date);//UTC date
	
	if(tmp_gpsinfo.status == 'A')
	{
		gpsNolocalTime = 0;
		
		if(latitude[0] == ' ')
		{
			latitude[0]= '0';
			if((latitude[4] != '.')||(Is_Digit(&latitude[1],3) == 0)||(Is_Digit(&latitude[5],4) == 0))
			{
				LOG("[gps] lat error\r\n");
				return;
			}
		}
		else
		{
			if((latitude[4] != '.')||(Is_Digit(latitude,4) == 0)||(Is_Digit(&latitude[5],4) == 0))
			{
				LOG("[gps] lat error\r\n");
				return;
			}
		}
		if((latitude_flag != 'S')&&(latitude_flag != 'N'))
		{
			return;
		}
	
		if((longitude[5] != '.')||(Is_Digit(longitude,5) == 0)||(Is_Digit(&longitude[6],4) == 0))
		{
			LOG("[gps] lon error\r\n");
			return;
		}
		if((longitude_flag != 'W')&&(longitude_flag != 'E'))
		{
			return;
		}

		tmp_gpsinfo.latitude_flag = latitude_flag;
		tmp_gpsinfo.latitude_str[0] = '0';
		strncpy_t(&tmp_gpsinfo.latitude_str[1], latitude, OA_NMEA_STR_LEN-1);

		tmp_gpsinfo.longitude_flag = longitude_flag;
		strncpy_t(&tmp_gpsinfo.longitude_str[0], longitude, OA_NMEA_STR_LEN-1);
	
		gps_date_utc_to_rtc(sys_GetTimeZone(), &t_date, &tmp_gpsinfo.date);

        /******************************************************************************/
        //静态漂移速度过滤(未定位-->定位)
        //过滤从未定位到定位时10s内的速度
        if(sys_state_get_gpslastloc() == 0)
        {
            sFilterSpeedTime = 0;
            sFilterSpeedFlag = 1;
        }
        if(sFilterSpeedFlag)
        {
            if(++sFilterSpeedTime < 10)
            {
				memset_t(tmp_gpsinfo.speed_str, 0, OA_NMEA_MIN_LEN);
                strcpy_t(tmp_gpsinfo.speed_str, "000.00");    
            }
            else
            {
                sFilterSpeedFlag = 0;    
            }
        }
		/******************************************************************************/

        memcpy_t(&g_gpsinfo,&tmp_gpsinfo,sizeof(gpsinfo));
		sys_SetSysRTC(&g_gpsinfo.date);	//rtc校时

		sys_state_gpsloc(1);			//定位状态
		gps_mileage_calc();				//里程统计
		gps_filter_speed();				//速度过滤

		//实时更新位置信息
		if(gps_update_location_condition() == TRUE)
		{
			sys_SetLastGpsData(&g_gpsinfo);
			gps_save_validPoint();//经纬度保存
		}
		//处理恢复出厂最后一次经纬度清0
		if(strstr_t(g_gpsLastinfo.latitude_str, "00000.0000") || strstr_t(g_gpsLastinfo.longitude_str, "00000.0000"))
		{
			sys_SetLastGpsData(&g_gpsinfo);
			gps_valid_save(&g_gpsLastinfo);
		}
	}
	else
	{   
		g_gpsinfo.status = 'V';

		strcpy_t(g_gpsinfo.latitude_str, "00000.0000");
		strcpy_t(g_gpsinfo.longitude_str, "00000.0000");
		strcpy_t(g_gpsinfo.speed_str, "00.00");
		strcpy_t(g_gpsinfo.head_str, "00.00");

		sys_state_gpsloc(0);
		
		//不定位5min复位一次
		if(gpsNolocalTime++ >= 300)
		{
			LOG("[gps] 5min no fix reset...\r\n");
			gps_reset();
			gpsNolocalTime = 0;		
		}
	}
	sys_state_offAlarm(ALARMST_GPSMODEM);	//取消模块故障
	sys_state_lastGpsA();					//最后定位状态标志
	
	
	#if 0
	LOG("[OA]NMEA: %c, <%c,%s>,<%c,%s>,%s,<%d-%d-%d %d-%d-%d>", g_gpsinfo.status,
            g_gpsinfo.latitude_flag, g_gpsinfo.latitude_str,
            g_gpsinfo.longitude_flag, g_gpsinfo.longitude_str,
            g_gpsinfo.speed_str,g_gpsinfo.date.year,g_gpsinfo.date.month,g_gpsinfo.date.day,
			g_gpsinfo.date.hour,g_gpsinfo.date.minute,g_gpsinfo.date.second);
	#endif
}
//$GPGGA
void gps_nmea_parse_gpgga(unsigned char *data, unsigned short len)
{
//$GPGGA,013122.000,2235.5344,N,11352.2559,E,1,06,2.0,119.6,M,-3.0,M,,0000*45
	unsigned char *p=data;
	unsigned short l=len;
	char star[4]={0};
    char hdop[8]={0};
	char high[16]={0};
	//报文校验
	if(gps_nmea_xor_check(p,len) == XOR_FAILED)
	{
		return;
	}
	//跳过头帧
	p += 7;
	l -= 7;

	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_skip_token(&p, &l, ',');
	gps_nmea_string_token(&p, &l, ',', star, 4);
    gps_nmea_string_token(&p, &l, ',', hdop, 8);
	gps_nmea_string_token(&p, &l, ',', high, 16);
	
	g_gpsinfo.sat_num = myatoi((char*)star);
    g_gpsinfo.hdop    = myatoi((char*)hdop);

	g_gps_star_num = g_gpsinfo.sat_num;
	if(strchr_t((char *)high, '-'))
	{
		g_gps_altitude = 0;
	}
	else
	{
		g_gps_altitude = myatoi((char*)high);
	}
}

//GPS模块故障检测
void gps_module_errCheck(void)
{	
	unsigned char errflag=0;
	static unsigned char errstep=0;

	if(g_flag.bit.GpsPowFlag == GPS_POWOFF)
	{
		gpsErrorTime   = 0;
		g_gps_star_num = 0;
		sys_state_gpsloc(0);
	}
	if(sys_state_get_gpsloc())
	{
		errstep = 0;
	}
	switch(errstep)
	{
		case 0:
			if(++gpsErrorTime > 1800)//30s
			{
				gpsErrorTime = 0;
				errflag = 1;
				errstep = 1;
			}
			break;
		case 1:
			if(++gpsErrorTime > 3600)//5min
			{
				gpsErrorTime = 0;
				errflag = 1;
				errstep = 1;
			}
			break;
		default:
			break;
	}
	//gps模块故障
	if(errflag == 1)
	{		
		LOG("[gps] module error reset...\r\n");
		errflag = 0;
		sys_state_newAlarm(ALARMST_GPSMODEM);
		sys_state_gpsloc(0);
		sys_clear_lastGpsA();
		g_gps_altitude = 0;
		g_gps_star_num = 0;
		g_gpsinfo.sat_num = 0;
        g_gpsinfo.hdop = 0;
		gps_reset();
	}
}


//定时监控
void gps_watch_gpstime(unsigned short n)
{
	static unsigned char start=0;
	static TIME LastTime;//时间基准点
	TIME NowTime;

	if(n == 0)
	{
		start = 0;
		return;	
	}

	if(start == 0)
	{
		start = 1;
		memcpy_t(&LastTime,sys_GetSysRTC(),sizeof(TIME));
	}
	memcpy_t(&NowTime,sys_GetSysRTC(),sizeof(TIME));
	if(rtc_check_time(&LastTime,&NowTime) >= n)
	{
		memcpy_t(&LastTime,&NowTime,sizeof(TIME));
		aw_send_location_msg();
	}
}

//gps监控处理
void gps_watch_proc(void)
{
	unsigned short interval=0;

	if(modem_GetState() == GSM_FOTA_STATE)//远程升级return
	{
		return;
	}
	if(sys_state_get_acc())//ACC ON
	{
		interval = sys_GetAccStateOnInterval();
	}
	else//ACC OFF
	{
		interval = sys_GetAccStateOffInterval();
	}
	gps_watch_gpstime(interval);
}

//gps报警检测处理
void gps_AlarmHandle(void)
{
	
}

//获取定位数据
gpsinfo  *gps_GetGpsInfo(void)
{
	return &g_gpsinfo;
}
//获取GPS速度
unsigned int gps_GetGpsSpeed(void)
{
    char speed[OA_NMEA_MIN_LEN+1]={0};
	
    memset_t((char*)speed, 0, sizeof(speed));
    memcpy_t((char*)speed, g_gpsinfo.speed_str, strlen_t((char*)g_gpsinfo.speed_str));
	
    return (unsigned int)((myatof(speed))*1.852f); 
}
//获取方向角
void gps_GetGpsHead(unsigned char *data)
{
    if (data == NULL)
    {
        return ;
    }

    memcpy_t(data,g_gpsinfo.head_str,sizeof(g_gpsinfo.head_str));
}
//获取日期时间
void gps_GetGpsTime(TIME *data)
{
	if (data == NULL)
    {
        return ;
    }

    memcpy_t(data,&g_gpsinfo.date,sizeof(g_gpsinfo.date));
}
//卫星数
unsigned char gps_GetGpsStarNum(void)
{
	return g_gps_star_num;
}

//高度
unsigned short gps_GetGpsAltitude(void)
{
	return g_gps_altitude;
}

//获取GPRMC
unsigned char *gps_GetGprmc(void)
{
	return sGPRMC;
}

//gps串口中断
void gps_recv(unsigned char *data, unsigned short len)
{
	RingBuffer_InsertMult(&GPSRINGBUFF, (unsigned char *)data, len);

	OS_SendMessageFromISR(xGpsQueueId, MSG_GPS_RECV_REQ, 0, 0);	
}

unsigned short gps_check_head(char* head, unsigned char* inbuf, unsigned char* outbuf)
{
	char *pStart = NULL;
	char *pEnd = NULL;
	unsigned short _return = 0;

	pStart = strstr_t((char *)inbuf, (char *)head);
	if(pStart)
	{
		pEnd = strstr_t((char *)pStart, "\r\n");
		if(pEnd)
		{
			if(pEnd-pStart < INPUT_GPS_LEN)
			{
				memcpy_t(outbuf, pStart, pEnd-pStart);
				outbuf[pEnd-pStart] = 0;

				_return = pEnd-pStart;
			}
		}
	}

	return _return;
}

//GPS报文解析
void gps_nmea_parse(unsigned char *gpsBuf)
{
	unsigned char i;
	// Show GPS data
	if(sys_get_debugSwitch() == (LOG_GPSINFO))
	{
		PRINTF("%s",gpsBuf);
	}

	for(i=0; i<MAX_GPS_TYPE; i++)
	{
		gps_check_head((char *)sGpsType[i], gpsBuf, szcGpsBuf[i]);
	}

	//GP
	if(strlen_t((char *)szcGpsBuf[0]))
	{
		gps_nmea_parse_gpgga(szcGpsBuf[0], strlen_t((char *)&szcGpsBuf[0]));
		memset_t((char *)szcGpsBuf[0], 0, INPUT_GPS_LEN);
	}
	if(strlen_t((char *)szcGpsBuf[1]))
	{
		gps_nmea_parse_gprmc(szcGpsBuf[1], strlen_t((char *)&szcGpsBuf[1]));
		memset_t((char *)szcGpsBuf[1], 0, INPUT_GPS_LEN);
	}
	//BD
	if(strlen_t((char *)szcGpsBuf[2]))
	{
		gps_nmea_parse_gpgga(szcGpsBuf[2], strlen_t((char *)&szcGpsBuf[2]));
		memset_t((char *)szcGpsBuf[2], 0, INPUT_GPS_LEN);
	}
	if(strlen_t((char *)szcGpsBuf[3]))
	{
		gps_nmea_parse_gprmc(szcGpsBuf[3], strlen_t((char *)&szcGpsBuf[3]));
		memset_t((char *)szcGpsBuf[3], 0, INPUT_GPS_LEN);
	}
	//GN
	if(strlen_t((char *)szcGpsBuf[4]))
	{
		gps_nmea_parse_gpgga(szcGpsBuf[4], strlen_t((char *)&szcGpsBuf[4]));
		memset_t((char *)szcGpsBuf[4], 0, INPUT_GPS_LEN);
	}
	if(strlen_t((char *)szcGpsBuf[5]))
	{
		gps_nmea_parse_gprmc(szcGpsBuf[5], strlen_t((char *)&szcGpsBuf[5]));
		memset_t((char *)szcGpsBuf[5], 0, INPUT_GPS_LEN);
	}	
}

void vGpsTaskQueueCreat(void)
{
	xGpsQueueId = xQueueCreate(5 , sizeof(OS_EVENT));
	if(xGpsQueueId == 0)
	{
		LOG("#gps task queue fail\r\n");
	}
}

void vGpsTask(void *pvParameters)
{
	OS_EVENT Msg;

	OS_Timer_Start(xGpsQueueId,MSG_GPS_TIME_REQ,0,0,GPS_TIMER_ID,100,TRUE);// 1s

	LOG("[task] gps start\r\n");
	
    for( ;; )
    {
		vGetStackHighWaterMark("GpsTask");

		OS_GetMessage(xGpsQueueId, &Msg);		
		switch(Msg.sEventType)
		{
			case MSG_GPS_RECV_REQ:
			{
				xGpsParameters *pUartGpsBuf = NULL;

				pUartGpsBuf = (xGpsParameters *)pvPortMalloc(sizeof(xGpsParameters));
				if(pUartGpsBuf == NULL)
				{
					return;
				}
				memset_t(pUartGpsBuf, 0, sizeof(xGpsParameters));

				pUartGpsBuf->usMsgLen = RingBuffer_GetCount(&GPSRINGBUFF);
				RingBuffer_PopMult(&GPSRINGBUFF, pUartGpsBuf->ucMsgRcvBuf, pUartGpsBuf->usMsgLen);
				
				gps_nmea_parse(pUartGpsBuf->ucMsgRcvBuf);
				
				vPortFree(pUartGpsBuf);
				gps_zkw_proc();
			}
			break;

			case MSG_GPS_TIME_REQ:
			{
				gps_AlarmHandle();		//报警处理
 				gps_watch_proc(); 		//定时监控
 				gps_module_errCheck();	//模块故障检测
				LOG("gps run free:%d\r\n",uxTaskGetStackHighWaterMark(NULL));
			}
			break;

			case MSG_GPS_POWOFF_REQ:
			{
				gps_powerOff();
				OS_Timer_Start(xGpsQueueId,MSG_GPS_POWON_REQ,0,0,GPS_RESET_TIMER_ID,1000,FALSE);// 10s			
			}
			break;

			case MSG_GPS_POWON_REQ:
			{
				gps_powerOn();
			}
			break;
			default:
			break;
		}
    }
}

