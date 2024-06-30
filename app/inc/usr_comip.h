#ifndef __USR_COMIP_H__
#define __USR_COMIP_H__

typedef enum
{
    IPV_TCP_DATA_SEG_HEADER,
	IPV_TCP_DATA_SEG_CMD,
	IPV_TCP_DATA_SEG_VIN,
	IPV_TCP_DATA_SEG_IMEI,
	IPV_TCP_DATA_SEG_VER,
	IPV_TCP_DATA_SEG_CRYP,
    IPV_TCP_DATA_SEG_LEN,
    IPV_TCP_DATA_SEG_RAWDATA,
    IPV_TCP_DATA_SEG_XOR,

    IPV_TCP_DATA_SEG_MAX
}ipv_tcp_protocol_data_segment;

//#define ON_STS_ACC					0x00000001//ACC
//#define ON_STS_GPSLOC				0x00000002//��λ
//#define ON_STS_GPSLAT				0x00000004//��1��0γ
//#define ON_STS_GPSLON				0x00000008//��0��1��
//#define ON_STS_GPS_FIX				0x00040000//GPS ���Ƕ�λ
//#define ON_STS_BD_FIX				0x00080000//BD ���Ƕ�λ
//#define ON_STS_GLONASS_FIX			0x00100000//GLONASS ���Ƕ�λ
//#define ON_STS_GALiLEO_FIX			0x00200000//Galileo ���Ƕ�λ

//#define ALARMST_POWER_DOWN			0x00000001//����Դ�ϵ籨��
//#define ALARMST_POWER_LOW			0x00000002//����ԴǷѹ

//#define ALARMST_GPSMODEM			0x00000010//GPSģ�����
//#define ALARMST_GPSANTO				0x00000020//GPS���߿�·
//#define ALARMST_GPSANTS				0x00000040//GPS���߶�·

/**********************************************************************************/
void modem_gsm_ipvip(void);
void modem_reconnect_ip2(void);


void modem_gsm_close_ip2(void);
void modem_gsm_disconnect_ip2(void);
void modem_gsm_reconnect_ip2(void);
void modem_gsm_ip2_errcheck(void);
void modem_gsm_Login2(void);
void modem_gsm_center2(void);
/**********************************************************************************/
void ipv_serialnum_noinit(void);
void ipv_login_msg(void);
void ipv_send_param_msg(void);
void ipv_send_location_msg2(void);
void ipv_recv_Handle(char* buf,unsigned short buflen);	
#endif
