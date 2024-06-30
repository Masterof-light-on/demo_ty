#ifndef __USR_AW_H__
#define __USR_AW_H__

typedef struct __GBROAD_MSGPACK__
{
	unsigned short total;		//��Ϣ������
	unsigned short cur_bag;	//��ǰ����
}Gbroad_msgpack;

typedef struct __GBROAD_MSG__
{
	unsigned short cmdid;		//��ϢID
	unsigned short msgattr;	//��Ϣ����
	unsigned short msgsel;		//��Ϣ��ˮ��
	unsigned char msgtype;		//0: gprs/cdmaģʽ;  1: smsģʽ
	Gbroad_msgpack msgpack;	//��Ϣ��װ��	
	
	unsigned char *msg;		//��Ϣ��
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

#define MSG_UP_ACK				0x80000000	//��Ӧ,�Դ���ˮ��
#define MSG_UP_GPS				0x40000000	//��GPS����
#define MSG_ENCODE				0x400		//���ݼ���
#define MSG_MULTY				0x2000		//���ݷְ�
#define MSG_LENINVALID			0x3FF		//������Ч����


//����״̬
//car sate  
#define ON_STS_ACC						0x00000001ul	//ACC
#define ON_STS_GPSLOC					0x00000002ul	//��λ
#define ON_STS_GPSLAT					0x00000004ul	//��1��0γ
#define ON_STS_GPSLON					0x00000008ul	//��0��1��
#define ON_STS_OPERATE					0x00000010ul	//��Ӫ״̬
#define ON_STS_GPSENCRYPT				0x00000020ul	//��γ�ȼ���״̬
#define REV1							0x00000040ul
#define REV2							0x00000080ul
#define ON_STS_CARSTATE1				0x00000100ul	//�ͳ��Ŀա��س��������Ŀ��ء�����״̬��ʾ
#define ON_STS_CARSTATE2				0x00000200ul
#define ON_STS_GAS_LOCK					0x00000400ul	//����·
#define ON_STS_POWEROUT					0x00000800ul	//�ϵ�(�����ⲿ)
#define ON_STS_DOOR_LOCK				0x00001000ul	//����״̬
#define ON_STS_DOOR1					0x00002000ul	//ǰ��
#define ON_STS_DOOR2					0x00004000ul	//����
#define ON_STS_DOOR3					0x00008000ul	//����
#define ON_STS_DOOR4					0x00010000ul	//��ʻ��
#define ON_STS_DOOR5					0x00020000ul	//�Զ�����
#define ON_STS_GPS_FIX					0x00040000ul	//GPS ���Ƕ�λ
#define ON_STS_BD_FIX					0x00080000ul	//BD ���Ƕ�λ
#define ON_STS_GLONASS_FIX				0x00100000ul	//GLONASS ���Ƕ�λ
#define ON_STS_GALiLEO_FIX				0x00200000ul	//Galileo ���Ƕ�λ


//alarm state
#define ALARMST_HIJACK					0x00000001ul //����
#define ALARMST_SPEED					0x00000002ul //���ٱ���
#define ALARMST_DRIVETIMEOUT			0x00000004ul //ƣ�ͼ�ʻ
#define ALARMST_NOTICE					0x00000008ul //Σ��Ԥ��
#define ALARMST_GPSMODEM				0x00000010ul //GPSģ�����
#define ALARMST_GPSANTO					0x00000020uL //GPS���߿�·
#define ALARMST_GPSANTS					0x00000040ul //GPS���߶�·
#define ALARMST_POWER_LOW				0x00000080ul //����ԴǷѹ
#define ALARMST_POWER_DOWN				0x00000100ul //����Դ�ϵ籨��
#define ALARMST_HANDLE_ERR				0x00000200ul //�ն���ʾ������
#define ALARMST_TTS_ERR					0x00000400ul //TTS����
#define ALARMST_CAM_ERR					0x00000800ul //����ͷ����
#define ALARMST_IC_ERR					0x00001000ul //��·����֤ IC ��ģ����� 
#define ALARMST_WARN_SPEED				0x00002000ul //����Ԥ��
#define ALARMST_WARN_DRIVETIMEOUT		0x00004000ul //ƣ�ͼ�ʻԤ�� 
#define ALARMST_DEFINE1					0x00008000ul //�Զ���1  --> �ߴ���
#define ALARMST_DEFINE2					0x00010000ul //�Զ���2
#define ALARMST_DEFINE3					0x00020000ul //�Զ���3
#define ALARMST_DRVTM_DAY				0x00040000ul //�����ۼƼ�ʻ��ʱ
#define ALARMST_STOP					0x00080000ul //ͣ������
#define ALARMST_REGION					0x00100000ul //��������
#define ALARMST_LINE					0x00200000ul //������·
#define ALARMST_LINE_TIME				0x00400000ul //·��ʱ���������
#define ALARMST_LINE_OFFECT				0x00800000ul //ƫ��·��
#define ALARMST_VSS_ERR					0x01000000ul //VSS����
#define ALARMST_OIL_ABNO				0x02000000ul //�����쳣
#define ALARMST_CAR_THREF				0x04000000ul //��������
#define ALARMST_INVALID_STARTUP			0x08000000ul //�Ƿ����
#define ALARMST_MOVE					0x10000000ul //λ��
#define ALARMST_CRASH					0x20000000ul //��ײԤ��
#define ALARMST_ROLLOVER				0x40000000ul //�෭Ԥ��
#define ALARMST_ILLEGAL_OPENDOOR		0x80000000ul //�Ƿ����ű���


//sys state
#define SYS_ST_DEFINE1					0x00008000ul //�Զ���1
#define SYS_ST_DEFINE2					0x00010000ul //�Զ���2
#define SYS_ST_DEFINE3					0x00020000ul //�Զ���3



//ϵͳ֧�ֵ�Э�����б�
//����
#define MSG_GPRS_TERGENACK				0x0001	//�ն�ͨ��Ӧ��
#define MSG_GPRS_KEEPALIVE				0x0002	//���� 
#define MSG_GPRS_TERLOGOUT				0x0003	//�ն�ע��
#define MSG_GPRS_TERLOGOUT1				0x0101	//�ն�ע��
#define MSG_GPRS_TERREG					0x0100	//�ն�ע��
#define MSG_GPRS_TERLOGIN				0x0102	//�ն˼�Ȩ,��¼
#define MSG_GPRS_CHECKACK				0x0104	//��ѯ�ն˲���Ӧ��
#define MSG_GPRS_CHECKACK_APPEND		0x0107	//��ѯ�ն˲���Ӧ��
#define MSG_GPRS_GPSMSG					0x0200	//λ����Ϣ�㱨
#define MSG_GPRS_ROLLCALLUP				0x0201	//λ����Ϣ��ѯӦ�� 
#define MSG_GPRS_CARCONTROLUP			0x0500	//��������Ӧ�� 
#define MSG_GPRS_BATCHUPLOAD			0x0704  //��λ���������ϴ�
#define MSG_GPRS_MEDIAEVENTUP			0x0800	//��ý���¼���Ϣ�ϴ� 		***********
#define MSG_GPRS_MEDIAMSGUP				0x0801	//��ý�������ϴ�			***********
#define MSG_GPRS_AWEXTUP				0x0FFE	//�ǰ���չӦ��
#define MSG_GPRS_GENERAL_UPLOAD			0x0900	//����͸��

//����������
#define MSG_GPRS_GENERAL_UPLOAD_LED		0x0001  //����LED���������
#define MSG_GPRS_GENERAL_UPLOAD_JJQ		0x0002  //�Ƽ�������
#define MSG_GPRS_GENERAL_UPLOAD_TEMP	0x0003  //�¶Ȳɼ�������
#define MSG_GPRS_GENERAL_UPLOAD_OIL		0x0004  //�ͺĲɼ�������
#define MSG_GPRS_GENERAL_UPLOAD_CAN		0x0005  //CAN����
#define MSG_GPRS_GENERAL_UPLOAD_CALL	0x0006  //���ٲ�ѯ


//����
#define MSG_GPRS_CENGENACK				0x8001	//ƽ̨ͨ��Ӧ��
#define MSG_GPRS_CENREGACK				0x8100	//�ն�ע��
#define MSG_GPRS_PARA_SET				0x8103	//�����ն˲���
#define MSG_GPRS_PARA_CHECK				0x8104	//��ѯ�ն˲���
#define MSG_GPRS_TERCONTRL				0x8105	//�ն˿���
#define MSF_GPRS_SPE_CHECK				0x8106  //��ѯָ���ն˲���
#define MSG_GPRS_TER_CHECK				0x8107	//��ѯ�ն�����
#define MSG_GPRS_ROLLCALL				0x8201	//λ����Ϣ��ѯ(����)
#define MSG_GPRS_SCHEDDOWN				0x8300	//�ı���Ϣ�·�
#define MSG_GPRS_PHONELISTEN			0x8400	//����
#define MSG_GPRS_CARCONTROL				0x8500	//��������
#define MSG_GPRS_REGENCYCLESET			0x8600	//����Բ������
#define MSG_GPRS_REEGNCYCLEDEL			0x8601	//ɾ��Բ������
#define MSG_GPRS_REGENRECTSET			0x8602	//���þ������� 			***********
#define MSG_GPRS_REGENRECTDEL			0x8603	//ɾ����������			***********
#define MSG_GPRS_REGENPOLYGONSET		0x8604	//���ö�������� 			***********
#define MSG_GPRS_REGENPOLYGONDEL		0x8605	//ɾ�����������	
#define MSG_GPRS_MEDIAMSGACK			0x8800	//��ý�������ϴ�Ӧ�� 	***********
#define MSG_GPRS_PICTAKE				0x8801	//����ͷ������������	***********
#define MSG_GPRS_MEDIAMSGCHECK			0x8802	//�洢��ý�����ݼ��� 	***********
#define MSG_GPRS_MEDIAMSGCHKS			0x8805	//�����洢��ý�����ݼ����ϴ�����
#define MSG_GPRS_AWVERSION				0x8702	//�ǰ��汾��
#define MSG_GPRS_AWEXTDOWN				0x8FFE	//�ǰ���չӦ��
#define MSG_GPRS_GENERAL_DOWNLOAD		0x8900	//����͸��

//����������
#define MSG_GPRS_GENERAL_DOWNLOAD_LED	0x8001  //����LED���������
#define MSG_GPRS_GENERAL_DOWNLOAD_JJQ	0x8002  //�Ƽ�������
#define MSG_GPRS_GENERAL_DOWNLOAD_TEMP	0x8003  //�¶Ȳɼ�������
#define MSG_GPRS_GENERAL_DOWNLOAD_OIL	0x8004  //�ͺĲɼ�������
#define MSG_GPRS_GENERAL_DOWNLOAD_CAN	0x8005  //CAN����
#define MSG_GPRS_GENERAL_DOWNLOAD_CALL	0x8006  //���ٲ�ѯ


//��չЭ��
#define MSG_GPRS_CHECK_VER				0x8006	//��ѯ�ն˰汾
#define MSG_GPRS_OIL_CONTROL			0x8008	//��·����
#define MSG_GPRS_OIL_VOLUME				0x8010  //�ͺ����
#define MSG_GPRS_OIL_TEMP				0x8016  //�ͺ���Ʈ
#define MSG_GPRS_OIL_DAMP				0x8018  //�ͺ�����


//0x8103 �����ն˲����ĸ������
#define MSG_GPRS_PARA_KEEPALIVE			0x0001	//�ն�����(s)

#define MSG_GPRS_PARA_APNMAIN			0x0010	//�������� APN
#define MSG_GPRS_PARA_USRMAIN			0x0011	//������������ͨ�Ų����û��� 
#define MSG_GPRS_PARA_PWDMAIN			0x0012	//������������ͨ�Ų�������
#define MSG_GPRS_PARA_IPMAIN			0x0013	//����������ַ,IP������
#define MSG_GPRS_PARA_APNSUB			0x0014	//���ݷ�����APN
#define MSG_GPRS_PARA_USRSUB			0x0015	//���ݷ���������ͨ�Ų����û���
#define MSG_GPRS_PARA_PWDSUB			0x0016	//���ݷ���������ͨ�Ų�������
#define MSG_GPRS_PARA_IPSUB				0x0017	//���ݷ�������ַ,IP������
#define MSG_GPRS_PARA_PORTTCP			0x0018	//������TCP�˿� 
#define MSG_GPRS_PARA_PORTUDP			0x0019	//������UDP�˿�

#define MSG_GPRS_PARA_WATTYPE			0x0020	//λ�û㱨����
#define MSG_GPRS_PARA_WATMODE			0x0021	//λ�û㱨����
#define MSG_GPRS_PARA_SLEEPTIMER		0x0027	//����ʱ�㱨ʱ����
#define MSG_GPRS_PARA_WATTIMEDEF		0x0029	//ȱʡʱ�Ļ㱨ʱ����(s)
#define MSG_GPRS_PARA_WATDISDEF         0x002C	//ȱʡʱ�Ļ㱨������(m)
#define MSG_GPRS_PARA_REDEEM         	0x0030	//�յ㲹���Ƕ�
#define MSG_GPRS_PARA_MOVE_RADIUS       0x0031  //����Χ���뾶���Ƿ�λ����ֵ��
#define MSG_GPRS_PARA_PHONERVTYPE		0x0045	//�ն˵绰��������
#define MSG_GPRS_PARA_PHONECEN			0x0040	//���ƽ̨�绰���� 
#define MSG_GPRS_PARA_PHONEREBOOT		0x0041	//��λ�绰����
#define MSG_GPRS_PARA_PHONERESET		0x0042	//�ָ��������õ绰����
#define MSG_GPRS_PARA_PHONECENSMS		0x0043	//���ƽ̨SMS �绰����
#define MSG_GPRS_PARA_PHONESMSALM		0x0044	//�����ն�SMS �ı���������
#define MSG_GPRS_PARA_PHONERVTYPE		0x0045	//�ն˵绰��������
#define MSG_GPRS_PARA_PHONERVTONCE		0x0046	//ÿ���ͨ��ʱ��(s)
#define MSG_GPRS_PARA_PHONERVTALL		0x0047	//�����ͨ��ʱ��(s)
#define MSG_GPRS_PARA_PHONELISTENED		0x0048	//�����绰����
#define MSG_GPRS_PARA_PHONERIGHT		0x0049	//���ƽ̨��Ȩ����
#define MSG_GPRS_PARA_ALMENABEL			0x0050	//����������
#define MSG_GPRS_PARA_ALMENSMS			0x0051	//���������ı�SMS����
#define MSG_GPRS_PARA_ALMENPIC			0x0052	//�������㿪��
#define MSG_GPRS_PARA_ALMENPICSAVE		0x0053	//��������洢��־
#define MSG_GPRS_PARA_ALMSPEEDOVER		0x0055	//����ٶ�(km/h)
#define MSG_GPRS_PARA_ALMSPEEDTIME		0x0056	//���ٳ���ʱ��(s)
#define MSG_GPRS_PARA_ALMDRVTIMEONCE	0x0057	//������ʻʱ������(s)
#define MSG_GPRS_PARA_ALMDRVRESET		0x0059	//��С��Ϣʱ��(s)
#define MSG_GPRS_PARA_ALMSTOPTIME		0x005A	//�ͣ��ʱ��(s)


#define MSG_GPRS_PARA_MILLAGE			0x0080	//������̱����(1/10km)
#define MSG_GPRS_PARA_PROVINCE			0x0081	//�������ڵ�ʡ��ID 
#define MSG_GPRS_PARA_CITY				0x0082	//�������ڵ�����ID 
#define MSG_GPRS_PARA_CARLICENCE		0x0083	//����
#define MSG_GPRS_PARA_CARLICENCECOL		0x0084	//������ɫ


//������ϢID����
#define GPRS_APPEND_Millage 			0x01	//���
#define GPRS_APPEND_Oil 				0x02	//����
#define GPRS_APPEND_Vehilce_Speed 		0x03	//��ʻ��¼���ܻ�ȡ���ٶ�
#define GPRS_APPEND_Comfirm_ID 			0x03	//��Ҫ�˹�ȷ�ϱ����¼��� ID
#define GPRS_APPEND_SpeedAlarm  		0x11	//���ٱ���
#define GPRS_APPEND_RegionAlarm			0x12	//��������/·�߱���������Ϣ
#define GPRS_APPEND_DriverAlarm			0x13	//·����ʻʱ�䲻��/��������
#define GPRS_APPEND_ExtVehilceState		0x25	//��չ�����ź�״̬λ
#define GPRS_APPEND_IOState				0x2A	//IO״̬λ
#define GPRS_APPEND_AD					0x2B	//ģ����
#define GPRS_APPEND_CSQ					0x30	//CSQ
#define GPRS_APPEND_GPS_STA				0x31	//GPS����

//���������־
#define GPS_REGION_IN					0x00	//��
#define GPS_REGION_OUT					0x01	//��

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
