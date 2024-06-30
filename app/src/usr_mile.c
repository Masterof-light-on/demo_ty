#include "includes.h"

static gps_block_t GpsMileagePost;	//gps里程值游标

//里程计算
unsigned int gps_Calc_Place(char* slat0,char*slon0,char*slat1,char*slon1)
{
#define RACE_REPRAE 1.0	//误差修正率

	Point A,B;
	unsigned int tmp;

	A = gps_MakePoint(slon0,slat0);
	B = gps_MakePoint(slon1,slat1);
	A.x = gps_Distance(A,B)*RACE_REPRAE;

	tmp = A.x*10;
	if(tmp%10 >= 5)
	{
		tmp += 10;
	}
	tmp /= 10;

	return tmp;
}
//经度，格式为dddmm.mmmm
//纬度, 格式为0ddmm.mmmm
//将GPS经纬度转为点坐标
Point gps_MakePoint(char const * const sLon,char const * const sLat)
{
	Point pt;
	
	pt.x = (sLon[0] - '0')*100.0+(sLon[1] - '0')*10.0+(sLon[2] - '0');
	pt.x += ((sLon[3] - '0')*10.0+(sLon[4] - '0')+(sLon[6] - '0')/10.0+(sLon[7] - '0')/100.0+\
		(sLon[8] - '0')/1000.0+(sLon[9] - '0')/10000.0+\
		((sLon[10]>=0x30&&sLon[10]<=0x39)?((sLon[10] - '0')/100000.0):0.0))/60.0;

	pt.y = (sLat[0] - '0')*100.0+(sLat[1] - '0')*10.0+(sLat[2] - '0');
	pt.y += ((sLat[3] - '0')*10.0+(sLat[4] - '0')+(sLat[6] - '0')/10.0+(sLat[7] - '0')/100.0+\
		(sLat[8] - '0')/1000.0+(sLat[9] - '0')/10000.0+\
		((sLat[10]>=0x30&&sLat[10]<=0x39)?((sLat[10] - '0')/100000.0):0.0))/60.0;	
	return pt;
}
double gps_Distance(Point A,Point B)
{
	//d(x1,y1,x2,y2)=r*arccos(sin(x1)*sin(x2)+cos(x1)*cos(x2)*cos(y1-y2))
	//x1,y1是纬度\经度的弧度单位，r为地球半径6371004
	//角度转成弧度
	
	double dMileageData;
	double radLat1 =  A.y * PI / 180.0;
	double radLat2 =  B.y * PI / 180.0;
	double a = radLat1 - radLat2;
	double b = (A.x - B.x)*PI/180.0;

	dMileageData = 10000*EARTH_RADIUS*2*asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
	return dMileageData;
}

//两点经纬度里程值的计算
void gps_mileage_calc(void)
{
	unsigned int mileage=0;
	unsigned char pop=0; 
	gpsinfo *lastgpsdata=NULL;

	//实时更新里程
	if(gps_update_location_condition() == TRUE)
	{
		lastgpsdata = sys_GetLastGpsData();

		if(sys_state_get_gpslastloc())//上一点是否定位
		{
			mileage = gps_Calc_Place(g_gpsinfo.latitude_str,g_gpsinfo.longitude_str,lastgpsdata->latitude_str,lastgpsdata->longitude_str);	
			if(mileage > 50000)//过滤
			{
				LOG("[mileage] error\r\n");
				return;
			}
			//里程系数
			pop = sys_GetageNum();
			mileage = (unsigned int)(mileage*(pop/100.0));
			
			sys_AddMileAge(mileage);

			gps_mileage_save();
		}
	}
}

/*gps里程值，如果溢出返回１，否则返回０*/
int GpsMileagePostAdd(gps_block_t *pRecord)
{
	int ret=0;
	
	pRecord->offset += 4;
	if(pRecord->offset > (FLASH_SECTOR_SIZE-1))
	{
		ret = 1;
	}
	return ret;
}

//每1s保存一次里程值
//用16页的flash(64kb)保存里程值
//保存方式为循环移动保存
//64kb /(1s*4) = 16384s = 4.55h
void gps_mileage_save(void)
{
	static unsigned int cmp_disatance=0xffffffff;
	unsigned int now_distance=0;
	unsigned char buf[4]={0};

    now_distance = sys_GetMileAge();
	if(cmp_disatance == now_distance)
	{
		return;
	}
    uint32_to_uint8(now_distance,buf);

    if(GpsMileagePost.offset > (FLASH_SECTOR_SIZE-1))
	{		
		flash_mileage_erase();
		GpsMileagePost.offset = 0;
	}

//	LOG("GPS write mileage offset:%d\r\n",GpsMileagePost.offset);
	
	flash_mileage_write(GpsMileagePost.sector+GpsMileagePost.offset,4,buf);//every time write 4bytes
	
	GpsMileagePost.offset += 4;

	cmp_disatance = now_distance;
}

//断电重启读取最后一次有效的里程值
void gps_mileage_read(void)
{
    unsigned char buf[4]={0};
	unsigned int now_distance=0;
	unsigned int last_distance=0;
	int sign=0;

	/*找出flash中写入新记录的位置*/
	GpsMileagePost.offset = 0;
	GpsMileagePost.sector = PAGE_MILEINFO_START;
	while(1)
	{        
		buf[0] = buf[1] = buf[2] = buf[3] = 0;
        flash_mileage_read(GpsMileagePost.sector+GpsMileagePost.offset,4,buf);//every time read 4bytes
		
		now_distance = uint8_to_uint32(buf);
		if(now_distance == 0xFFFFFFFF)
		{
			break;
		}
		if(1 == GpsMileagePostAdd(&GpsMileagePost))
		{
			sign = 1;
            last_distance = now_distance;
			break;
		}
		last_distance = now_distance;
	}
	
	if(1 == sign)
	{
		GpsMileagePost.offset = 0;
		GpsMileagePost.sector = PAGE_MILEINFO_START;
		flash_mileage_erase();
	}

//    LOG("GPS total mileage:%.1fKm , offset:%d\r\n",last_distance/1000.0,GpsMileagePost.offset);

	sys_SetMileAge(last_distance);
}

//new add
static gps_block_t GpsValidPost;

/*gps有效值，如果溢出返回１，否则返回０*/
int GpsValidPostAdd(gps_block_t *pRecord)
{
	int ret=0;
	
	pRecord->offset += SYS_PARAM_GPS_SIZE;
	if(pRecord->offset > (FLASH_SECTOR_GPSDATA_SIZE-1))
	{
		ret = 1;
	}
	return ret;
}

void gps_valid_save(gpsinfo *gpsValid)
{	
	unsigned char buf[SYS_PARAM_GPS_SIZE]={0};

	memcpy_t(buf, gpsValid, SYS_PARAM_GPS_SIZE);
	
    if(GpsValidPost.offset > (FLASH_SECTOR_GPSDATA_SIZE-1))
	{		
		flash_gpsvalid_erase();
		GpsValidPost.offset = 0;
	}
	
	flash_gpsvalid_write(GpsValidPost.sector+GpsValidPost.offset,SYS_PARAM_GPS_SIZE,buf);
	
	GpsValidPost.offset += SYS_PARAM_GPS_SIZE;
}

//断电重启读取最后一次有效值
void gps_valid_read(void)
{
    unsigned char buf[SYS_PARAM_GPS_SIZE]={0};
	int sign=0;
	
	/*找出flash中写入新记录的位置*/
	GpsValidPost.offset = 0;
	GpsValidPost.sector = PAGE_GPSDATA_START;
	while(1)
	{        
		memset_t(buf,0,SYS_PARAM_GPS_SIZE);
        flash_gpsvalid_read(GpsValidPost.sector+GpsValidPost.offset,SYS_PARAM_GPS_SIZE,buf);
		
		if((0xff == buf[0])&&(0xff == buf[1])&&(0xff == buf[2])&&(0xff == buf[3]))
		{
			if(sign == 0)
			{
				g_gpsLastinfo.date.year   = 0;
				g_gpsLastinfo.date.month  = 0;
				g_gpsLastinfo.date.day    = 0;
				g_gpsLastinfo.date.hour   = 0;
				g_gpsLastinfo.date.minute = 0;
				g_gpsLastinfo.date.second = 0;
				strcpy_t(g_gpsLastinfo.latitude_str, "00000.0000");
				strcpy_t(g_gpsLastinfo.longitude_str, "00000.0000");
				strcpy_t(g_gpsLastinfo.speed_str, "00.00");
				strcpy_t(g_gpsLastinfo.head_str, "00.00");
			}
			break;
		}
		if(1 == GpsValidPostAdd(&GpsValidPost))
		{
			sign = 1;
			break;
		}
		sign = 2;
		memcpy_t(&g_gpsLastinfo, buf, SYS_PARAM_GPS_SIZE);
	}
	
	if(1 == sign)
	{
		GpsValidPost.offset = 0;
		GpsValidPost.sector = PAGE_GPSDATA_START;
		flash_gpsvalid_erase();
	}
	RTC_Set_Time(&g_gpsLastinfo.date);
	LOG("[rtc] set time:%d-%d-%d %02d:%02d:%02d\r\n",g_gpsLastinfo.date.year,g_gpsLastinfo.date.month,g_gpsLastinfo.date.day,g_gpsLastinfo.date.hour,g_gpsLastinfo.date.minute,g_gpsLastinfo.date.second);
}

//每60s保存一个点
void gps_save_validPoint(void)
{
	static unsigned char stime=0;

	if(++stime >= 60)
	{
		stime = 0;
		gps_valid_save(&g_gpsLastinfo);
	}	
}


