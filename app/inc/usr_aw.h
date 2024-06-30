#ifndef __USR_AW_H__
#define __USR_AW_H__

typedef struct __GBROAD_MSGPACK__
{
	unsigned short total;		//消息包总数
	unsigned short cur_bag;	//当前包数
}Gbroad_msgpack;

typedef struct __GBROAD_MSG__
{
	unsigned short cmdid;		//消息ID
	unsigned short msgattr;	//消息属性
	unsigned short msgsel;		//消息流水号
	unsigned char msgtype;		//0: gprs/cdma模式;  1: sms模式
	Gbroad_msgpack msgpack;	//消息封装项	
	
	unsigned char *msg;		//消息体
}Gbroad_msg;

typedef struct __MSG_TLV__
{
	unsigned int type;
	unsigned short len;
	unsigned char *msg;
}TLV;


#define GBROAD_MSG_SIGN			0x7E

#define MSG_NEED_RETURN			0x80
#define MSG_IS_OK				0x00
#define MSG_IS_FAIL				0x01
#define MSG_IS_ERR				(0x02|MSG_NEED_RETURN)
#define MSG_IS_NOT_SUPORT		(0x03|MSG_NEED_RETURN)

#define MSG_UP_ACK				0x80000000	//回应,自带流水号
#define MSG_UP_GPS				0x40000000	//带GPS报文
#define MSG_ENCODE				0x400		//数据加密
#define MSG_MULTY				0x2000		//数据分包
#define MSG_LENINVALID			0x3FF		//数据有效长度


//工作状态
//car sate  
#define ON_STS_ACC						0x00000001ul	//ACC
#define ON_STS_GPSLOC					0x00000002ul	//定位
#define ON_STS_GPSLAT					0x00000004ul	//南1北0纬
#define ON_STS_GPSLON					0x00000008ul	//东0西1经
#define ON_STS_OPERATE					0x00000010ul	//运营状态
#define ON_STS_GPSENCRYPT				0x00000020ul	//经纬度加密状态
#define REV1							0x00000040ul
#define REV2							0x00000080ul
#define ON_STS_CARSTATE1				0x00000100ul	//客车的空、重车及货车的空载、满载状态表示
#define ON_STS_CARSTATE2				0x00000200ul
#define ON_STS_GAS_LOCK					0x00000400ul	//断油路
#define ON_STS_POWEROUT					0x00000800ul	//断电(控制外部)
#define ON_STS_DOOR_LOCK				0x00001000ul	//车门状态
#define ON_STS_DOOR1					0x00002000ul	//前门
#define ON_STS_DOOR2					0x00004000ul	//中门
#define ON_STS_DOOR3					0x00008000ul	//后门
#define ON_STS_DOOR4					0x00010000ul	//驾驶门
#define ON_STS_DOOR5					0x00020000ul	//自定义门
#define ON_STS_GPS_FIX					0x00040000ul	//GPS 卫星定位
#define ON_STS_BD_FIX					0x00080000ul	//BD 卫星定位
#define ON_STS_GLONASS_FIX				0x00100000ul	//GLONASS 卫星定位
#define ON_STS_GALiLEO_FIX				0x00200000ul	//Galileo 卫星定位


//alarm state
#define ALARMST_HIJACK					0x00000001ul //紧急
#define ALARMST_SPEED					0x00000002ul //超速报警
#define ALARMST_DRIVETIMEOUT			0x00000004ul //疲劳驾驶
#define ALARMST_NOTICE					0x00000008ul //危险预警
#define ALARMST_GPSMODEM				0x00000010ul //GPS模块故障
#define ALARMST_GPSANTO					0x00000020uL //GPS天线开路
#define ALARMST_GPSANTS					0x00000040ul //GPS天线短路
#define ALARMST_POWER_LOW				0x00000080ul //主电源欠压
#define ALARMST_POWER_DOWN				0x00000100ul //主电源断电报警
#define ALARMST_HANDLE_ERR				0x00000200ul //终端显示屏故障
#define ALARMST_TTS_ERR					0x00000400ul //TTS故障
#define ALARMST_CAM_ERR					0x00000800ul //摄像头故障
#define ALARMST_IC_ERR					0x00001000ul //道路运输证 IC 卡模块故障 
#define ALARMST_WARN_SPEED				0x00002000ul //超速预警
#define ALARMST_WARN_DRIVETIMEOUT		0x00004000ul //疲劳驾驶预警 
#define ALARMST_DEFINE1					0x00008000ul //自定义1  --> 高触发
#define ALARMST_DEFINE2					0x00010000ul //自定义2
#define ALARMST_DEFINE3					0x00020000ul //自定义3
#define ALARMST_DRVTM_DAY				0x00040000ul //当天累计驾驶超时
#define ALARMST_STOP					0x00080000ul //停车报警
#define ALARMST_REGION					0x00100000ul //进出区域
#define ALARMST_LINE					0x00200000ul //进出线路
#define ALARMST_LINE_TIME				0x00400000ul //路段时间过长或不足
#define ALARMST_LINE_OFFECT				0x00800000ul //偏离路线
#define ALARMST_VSS_ERR					0x01000000ul //VSS故障
#define ALARMST_OIL_ABNO				0x02000000ul //油量异常
#define ALARMST_CAR_THREF				0x04000000ul //车辆被盗
#define ALARMST_INVALID_STARTUP			0x08000000ul //非法点火
#define ALARMST_MOVE					0x10000000ul //位移
#define ALARMST_CRASH					0x20000000ul //碰撞预警
#define ALARMST_ROLLOVER				0x40000000ul //侧翻预警
#define ALARMST_ILLEGAL_OPENDOOR		0x80000000ul //非法开门报警


//sys state
#define SYS_ST_DEFINE1					0x00008000ul //自定义1
#define SYS_ST_DEFINE2					0x00010000ul //自定义2
#define SYS_ST_DEFINE3					0x00020000ul //自定义3



//系统支持的协议字列表
//上行
#define MSG_GPRS_TERGENACK				0x0001	//终端通用应答
#define MSG_GPRS_KEEPALIVE				0x0002	//心跳 
#define MSG_GPRS_TERLOGOUT				0x0003	//终端注销
#define MSG_GPRS_TERLOGOUT1				0x0101	//终端注销
#define MSG_GPRS_TERREG					0x0100	//终端注册
#define MSG_GPRS_TERLOGIN				0x0102	//终端鉴权,登录
#define MSG_GPRS_CHECKACK				0x0104	//查询终端参数应答
#define MSG_GPRS_CHECKACK_APPEND		0x0107	//查询终端参数应答
#define MSG_GPRS_GPSMSG					0x0200	//位置信息汇报
#define MSG_GPRS_ROLLCALLUP				0x0201	//位置信息查询应答 
#define MSG_GPRS_CARCONTROLUP			0x0500	//车辆控制应答 
#define MSG_GPRS_BATCHUPLOAD			0x0704  //定位数据批量上传
#define MSG_GPRS_MEDIAEVENTUP			0x0800	//多媒体事件信息上传 		***********
#define MSG_GPRS_MEDIAMSGUP				0x0801	//多媒体数据上传			***********
#define MSG_GPRS_AWEXTUP				0x0FFE	//星安扩展应用
#define MSG_GPRS_GENERAL_UPLOAD			0x0900	//上行透传

//上行子命令
#define MSG_GPRS_GENERAL_UPLOAD_LED		0x0001  //博海LED广告屏数据
#define MSG_GPRS_GENERAL_UPLOAD_JJQ		0x0002  //计价器数据
#define MSG_GPRS_GENERAL_UPLOAD_TEMP	0x0003  //温度采集器数据
#define MSG_GPRS_GENERAL_UPLOAD_OIL		0x0004  //油耗采集器数据
#define MSG_GPRS_GENERAL_UPLOAD_CAN		0x0005  //CAN数据
#define MSG_GPRS_GENERAL_UPLOAD_CALL	0x0006  //电召查询


//下行
#define MSG_GPRS_CENGENACK				0x8001	//平台通用应答
#define MSG_GPRS_CENREGACK				0x8100	//终端注册
#define MSG_GPRS_PARA_SET				0x8103	//设置终端参数
#define MSG_GPRS_PARA_CHECK				0x8104	//查询终端参数
#define MSG_GPRS_TERCONTRL				0x8105	//终端控制
#define MSF_GPRS_SPE_CHECK				0x8106  //查询指定终端参数
#define MSG_GPRS_TER_CHECK				0x8107	//查询终端属性
#define MSG_GPRS_ROLLCALL				0x8201	//位置信息查询(点名)
#define MSG_GPRS_SCHEDDOWN				0x8300	//文本信息下发
#define MSG_GPRS_PHONELISTEN			0x8400	//监听
#define MSG_GPRS_CARCONTROL				0x8500	//车辆控制
#define MSG_GPRS_REGENCYCLESET			0x8600	//设置圆形区域
#define MSG_GPRS_REEGNCYCLEDEL			0x8601	//删除圆形区域
#define MSG_GPRS_REGENRECTSET			0x8602	//设置矩形区域 			***********
#define MSG_GPRS_REGENRECTDEL			0x8603	//删除矩形区域			***********
#define MSG_GPRS_REGENPOLYGONSET		0x8604	//设置多边形区域 			***********
#define MSG_GPRS_REGENPOLYGONDEL		0x8605	//删除多边形区域	
#define MSG_GPRS_MEDIAMSGACK			0x8800	//多媒体数据上传应答 	***********
#define MSG_GPRS_PICTAKE				0x8801	//摄像头立即拍摄命令	***********
#define MSG_GPRS_MEDIAMSGCHECK			0x8802	//存储多媒体数据检索 	***********
#define MSG_GPRS_MEDIAMSGCHKS			0x8805	//单条存储多媒体数据检索上传命令
#define MSG_GPRS_AWVERSION				0x8702	//星安版本号
#define MSG_GPRS_AWEXTDOWN				0x8FFE	//星安扩展应用
#define MSG_GPRS_GENERAL_DOWNLOAD		0x8900	//下行透传

//下行子命令
#define MSG_GPRS_GENERAL_DOWNLOAD_LED	0x8001  //博海LED广告屏数据
#define MSG_GPRS_GENERAL_DOWNLOAD_JJQ	0x8002  //计价器数据
#define MSG_GPRS_GENERAL_DOWNLOAD_TEMP	0x8003  //温度采集器数据
#define MSG_GPRS_GENERAL_DOWNLOAD_OIL	0x8004  //油耗采集器数据
#define MSG_GPRS_GENERAL_DOWNLOAD_CAN	0x8005  //CAN数据
#define MSG_GPRS_GENERAL_DOWNLOAD_CALL	0x8006  //电召查询


//扩展协议
#define MSG_GPRS_CHECK_VER				0x8006	//查询终端版本
#define MSG_GPRS_OIL_CONTROL			0x8008	//油路控制
#define MSG_GPRS_OIL_VOLUME				0x8010  //油耗体积
#define MSG_GPRS_OIL_TEMP				0x8016  //油耗温飘
#define MSG_GPRS_OIL_DAMP				0x8018  //油耗阻尼


//0x8103 设置终端参数的各命令定义
#define MSG_GPRS_PARA_KEEPALIVE			0x0001	//终端心跳(s)

#define MSG_GPRS_PARA_APNMAIN			0x0010	//主服务器 APN
#define MSG_GPRS_PARA_USRMAIN			0x0011	//主服务器无线通信拨号用户名 
#define MSG_GPRS_PARA_PWDMAIN			0x0012	//主服务器无线通信拨号密码
#define MSG_GPRS_PARA_IPMAIN			0x0013	//主服务器地址,IP或域名
#define MSG_GPRS_PARA_APNSUB			0x0014	//备份服务器APN
#define MSG_GPRS_PARA_USRSUB			0x0015	//备份服务器无线通信拨号用户名
#define MSG_GPRS_PARA_PWDSUB			0x0016	//备份服务器无线通信拨号密码
#define MSG_GPRS_PARA_IPSUB				0x0017	//备份服务器地址,IP或域名
#define MSG_GPRS_PARA_PORTTCP			0x0018	//服务器TCP端口 
#define MSG_GPRS_PARA_PORTUDP			0x0019	//服务器UDP端口

#define MSG_GPRS_PARA_WATTYPE			0x0020	//位置汇报策略
#define MSG_GPRS_PARA_WATMODE			0x0021	//位置汇报方案
#define MSG_GPRS_PARA_SLEEPTIMER		0x0027	//休眠时汇报时间间隔
#define MSG_GPRS_PARA_WATTIMEDEF		0x0029	//缺省时的汇报时间间隔(s)
#define MSG_GPRS_PARA_WATDISDEF         0x002C	//缺省时的汇报距离间隔(m)
#define MSG_GPRS_PARA_REDEEM         	0x0030	//拐点补传角度
#define MSG_GPRS_PARA_MOVE_RADIUS       0x0031  //电子围栏半径（非法位移阈值）
#define MSG_GPRS_PARA_PHONERVTYPE		0x0045	//终端电话接听策略
#define MSG_GPRS_PARA_PHONECEN			0x0040	//监控平台电话号码 
#define MSG_GPRS_PARA_PHONEREBOOT		0x0041	//复位电话号码
#define MSG_GPRS_PARA_PHONERESET		0x0042	//恢复出厂设置电话号码
#define MSG_GPRS_PARA_PHONECENSMS		0x0043	//监控平台SMS 电话号码
#define MSG_GPRS_PARA_PHONESMSALM		0x0044	//接收终端SMS 文本报警号码
#define MSG_GPRS_PARA_PHONERVTYPE		0x0045	//终端电话接听策略
#define MSG_GPRS_PARA_PHONERVTONCE		0x0046	//每次最长通话时间(s)
#define MSG_GPRS_PARA_PHONERVTALL		0x0047	//当月最长通话时间(s)
#define MSG_GPRS_PARA_PHONELISTENED		0x0048	//监听电话号码
#define MSG_GPRS_PARA_PHONERIGHT		0x0049	//监管平台特权号码
#define MSG_GPRS_PARA_ALMENABEL			0x0050	//报警屏蔽字
#define MSG_GPRS_PARA_ALMENSMS			0x0051	//报警发送文本SMS开关
#define MSG_GPRS_PARA_ALMENPIC			0x0052	//报警拍摄开关
#define MSG_GPRS_PARA_ALMENPICSAVE		0x0053	//报警拍摄存储标志
#define MSG_GPRS_PARA_ALMSPEEDOVER		0x0055	//最高速度(km/h)
#define MSG_GPRS_PARA_ALMSPEEDTIME		0x0056	//超速持续时间(s)
#define MSG_GPRS_PARA_ALMDRVTIMEONCE	0x0057	//连续驾驶时间门限(s)
#define MSG_GPRS_PARA_ALMDRVRESET		0x0059	//最小休息时间(s)
#define MSG_GPRS_PARA_ALMSTOPTIME		0x005A	//最长停车时间(s)


#define MSG_GPRS_PARA_MILLAGE			0x0080	//车辆里程表读数(1/10km)
#define MSG_GPRS_PARA_PROVINCE			0x0081	//车辆所在的省域ID 
#define MSG_GPRS_PARA_CITY				0x0082	//车辆所在的市域ID 
#define MSG_GPRS_PARA_CARLICENCE		0x0083	//车牌
#define MSG_GPRS_PARA_CARLICENCECOL		0x0084	//车牌颜色


//附加信息ID定义
#define GPRS_APPEND_Millage 			0x01	//里程
#define GPRS_APPEND_Oil 				0x02	//油量
#define GPRS_APPEND_Vehilce_Speed 		0x03	//行驶记录功能获取的速度
#define GPRS_APPEND_Comfirm_ID 			0x03	//需要人工确认报警事件的 ID
#define GPRS_APPEND_SpeedAlarm  		0x11	//超速报警
#define GPRS_APPEND_RegionAlarm			0x12	//进出区域/路线报警附加信息
#define GPRS_APPEND_DriverAlarm			0x13	//路段行驶时间不足/过长报警
#define GPRS_APPEND_ExtVehilceState		0x25	//扩展车辆信号状态位
#define GPRS_APPEND_IOState				0x2A	//IO状态位
#define GPRS_APPEND_AD					0x2B	//模拟量
#define GPRS_APPEND_CSQ					0x30	//CSQ
#define GPRS_APPEND_GPS_STA				0x31	//GPS星数

//区域进出标志
#define GPS_REGION_IN					0x00	//进
#define GPS_REGION_OUT					0x01	//出

#define GPS_REGION_SPEED_ROUND			0x01
#define GPS_REGION_SPEED_REC			0x02
#define GPS_REGION_SPEED_MUL			0x03
#define GPS_REGION_SPEED_ROAD			0x04

bool aw_udp_hangup(void);
bool aw_udp_keepalive(void);
void aw_send_location_msg(void);
void aw_send_rfid_msg(unsigned char type);
void aw_send_alarm_msg(void);
int oa_JT_check_login(char *msg,int len);
int oa_JT_query_filter_msg(unsigned char *msg, unsigned short msglen);
void oa_trac_send_general_msg(unsigned short type,unsigned char *pin,unsigned short len);
unsigned short oa_JT_create_TLV_AWExt(unsigned short type,unsigned char *pin,unsigned short len,unsigned char *pout);
void oa_recv_Handle(char* buf,unsigned short buflen);

#endif
