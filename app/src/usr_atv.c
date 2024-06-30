#include "includes.h"

//AT@
//SN
int pc_cmd_sn(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		sprintf((char *)out, "AT@SN?\r\n@SN:%s\r\nCMD_OK\r\n", sys_GetTermSN());
	}
	else if(str[0] == '=')
	{
		str ++;
		if(strlen_t((char *)str) != 17)
		{
			return 0;
		}
		
		sys_SetTermSN(str);
		sys_SetCarVin(str);

		sys_saveSN();
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;

		sprintf((char *)out, "AT@SN=%s\r\nCMD_OK\r\n", sys_GetTermSN());
	}
	else
	{
		return 0;
	}

	return 1;
}

//VIN
int pc_cmd_vin(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		sprintf((char *)out, "AT@VIN?\r\n@VIN:%s\r\nCMD_OK\r\n", sys_GetCarVin());
	}
	else if(str[0] == '=')
	{
		str ++;
		if(strlen_t((char *)str) != 17)
		{
			return 0;
		}
		
		sys_SetCarVin(str);

		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;

		sprintf((char *)out, "AT@VIN=%s\r\nCMD_OK\r\n", sys_GetCarVin());
	}
	else
	{
		return 0;
	}

	return 1;
}

//IMEI
int pc_cmd_imei(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		if(strlen_t((char *)modem_GetImei()))
		{
			sprintf((char *)out, "AT@IMEI?\r\n@IMEI:%s\r\nCMD_OK\r\n", modem_GetImei());
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

//ICCID
int pc_cmd_iccid(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		if(strlen_t((char *)modem_GetCcid()))
		{
			sprintf((char *)out, "AT@ICCID?\r\n@ICCID:%s\r\nCMD_OK\r\n", modem_GetCcid());
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

//swver
int pc_cmd_swver(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		sprintf((char *)out, "AT@SWVER?\r\n@SWVER:%s\r\nCMD_OK\r\n", APP_VER);
	}
	else
	{
		return 0;
	}

	return 1;
}

//server
int pc_cmd_server(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;
	unsigned char buf[80] = {0};

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '?')
	{
		sprintf((char *)out, "AT@Server?\r\n@Server:%s%s,%s%d,%s,%s,%s\r\nCMD_OK\r\n", 
					sys_GetUrl(),sys_GetIp(),sys_GetLinkType()?"T":"U",sys_GetPort(),\
					sys_GetApn(),sys_GetApnUser(),sys_GetApnPasswd());
	}
	else if(str[0] == '=')
	{
		str ++;
		if(GetItemCount((char *)str, ',') != 4)
		{
			return 0;
		}

		memset_t(buf, 0, 80);
		GetStringPartEx((const char **)&str, (char *)buf, 80, ',');
		if(is_ip_address((char *)buf,strlen_t((char *)buf)) == 0)
		{
			sys_ClearUrl();
			sys_SetIp(buf);
		}
		else
		{
			sys_ClrIp();
			sys_SetUrl(buf);
		}
		str ++;

		memset_t(buf, 0, 80);
		GetStringPartEx((const char **)&str, (char *)buf, 80, ',');
		if(buf[0] == 'T')
		{
			sys_SetLinkType(1);			
		}
		else
		{
			sys_SetLinkType(0);
		}
		sys_SetPort(myatoi((char *)&buf[1]));
		str ++;

		memset_t(buf, 0, 80);
		GetStringPartEx((const char **)&str, (char *)buf, 80, ',');
		sys_SetApn(buf);
		str ++;

		memset_t(buf, 0, 80);
		GetStringPartEx((const char **)&str, (char *)buf, 80, ',');
		sys_SetApnUser(buf);
		str ++;

		sys_SetApnPasswd(str);

		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;

		sprintf((char *)out, "AT@Server=%s%s,%s%d,%s,%s,%s\r\nCMD_OK\r\n",
					sys_GetUrl(),sys_GetIp(),sys_GetLinkType()?"T":"U",sys_GetPort(),\
					sys_GetApn(),sys_GetApnUser(),sys_GetApnPasswd());
	}
	else
	{
		return 0;
	}

	return 1;
}

//CONFIG
int pc_cmd_config(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;
	unsigned char ret = 0;
	unsigned char rbuf[320]={0};

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(str[0] == '=')
	{
		str ++;
		if(strstr_t((char *)str, "*F10#") == NULL)
		{
			return 0;
		}
		str += 5;

		sprintf((char *)out, "AT@Config=*F10#%s\r\nCMD_OK\r\n\0", str);
		
		sSave_flag  = 0;
		sReset_flag = 0;
		sReppp_flag = 0;

		ret = handset_all_command(str,strlen_t((char *)str),rbuf);
		if((ret == 0) || (strlen_t((char *)rbuf) == 0))
		{
			return 0;
		}
		if(sSave_flag & SAVE_RUN_FLAG)
		{
			sys_saveRuning();
		}
		if(sSave_flag & SAVE_SYS_FLAG)
		{
			sys_saveSys();
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

//TEST
int pc_cmd_sim(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	
	if(strlen_t((char *)modem_GetCcid()))
	{
		sprintf((char *)out, "@SIM:1\r\nCMD_OK\r\n");
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_csq(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	if((modem_GetCsq() > 0)&&(modem_GetCsq() < 32))
	{
		sprintf((char *)out, "@CSQ:\"%d\"\r\nCMD_OK\r\n", modem_GetCsq());
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_flash(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;
	unsigned short ID;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	ID = SPI_Flash_ReadID();
	if((ID == 0xEF15)||(ID == 0xEF16)||(ID == 0xEF17)||(ID == 0x6814)||(ID == 0x6815)||(ID == 0x6816))
	{
		sprintf((char *)out, "@FLASH:1\r\nCMD_OK\r\n");
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_cgreg(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;
	unsigned char cgreg;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	cgreg = modem_GetCgreg();
	if((cgreg == 1)||(cgreg == 5))
	{
		sprintf((char *)out, "@CGREG:1\r\nCMD_OK\r\n");
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_power(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;
	

	return 1;
}

int pc_cmd_tcp(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	if(g_flag.bit.socketPPPFlag)
	{
		sprintf((char *)out, "@TCP:6\r\nCMD_OK\r\n");
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_gps(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	if(sys_state_get_gpsloc())
	{
		sprintf((char *)out, "@GPS:A\r\nCMD_OK\r\n");
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_stat(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	if(sys_state_get_gpsloc())
	{
		sprintf((char *)out, "@START:\"%d\"\r\nCMD_OK\r\n", gps_GetGpsStarNum());
	}
	else
	{
		return 0;
	}

	return 1;
}

int pc_cmd_acc(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;

	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	if(sys_state_get_acc())
	{
		sprintf((char *)out, "@ACCON:1\r\nCMD_OK\r\n");
	}
	else
	{
		sprintf((char *)out, "@ACCOFF:1\r\nCMD_OK\r\n");
	}

	return 1;
}


int pc_cmd_log(unsigned char *str, unsigned char *out)
{
	unsigned char *pEnd = NULL;
	unsigned char log=0;
	pEnd = (unsigned char *)strstr_t((char *)str, "\r\n");
	if(pEnd == NULL)
	{
		return 0;
	}
	*pEnd = 0;

	log = str[0] - '0';
	sys_set_debugSwitch(log);
	strcpy_t((char *)out, "LOG OK");
	return 1;
}

typedef struct
{
	unsigned char  *cmd;
	int (*cmd_handler)(unsigned char *str, unsigned char *out);
}CMD_T;

static CMD_T pc_cmd_proc[] =
{
	{(unsigned char*)"SN",		pc_cmd_sn},
	{(unsigned char*)"VIN",		pc_cmd_vin},
	{(unsigned char*)"IMEI",	pc_cmd_imei},
	{(unsigned char*)"ICCID",	pc_cmd_iccid},
	{(unsigned char*)"SWVER",	pc_cmd_swver},
	{(unsigned char*)"Server",	pc_cmd_server},
	{(unsigned char*)"Config",	pc_cmd_config},
	{(unsigned char*)"SIM",		pc_cmd_sim},
	{(unsigned char*)"CSQ",		pc_cmd_csq},
	{(unsigned char*)"FLASH",	pc_cmd_flash},
	{(unsigned char*)"CGREG",	pc_cmd_cgreg},
	{(unsigned char*)"POWER",	pc_cmd_power},
	{(unsigned char*)"TCP",		pc_cmd_tcp},
	{(unsigned char*)"GPS",		pc_cmd_gps},
	{(unsigned char*)"STAT",	pc_cmd_stat},
	{(unsigned char*)"ACC",		pc_cmd_acc},
	{(unsigned char*)"LOG",		pc_cmd_log},
};

static const int NEW_CMD = (sizeof(pc_cmd_proc)/sizeof(pc_cmd_proc[0]));

int handset_all_pccmd(unsigned char *str, unsigned short len, unsigned char *out)
{
	unsigned char *pCmd=NULL;
	unsigned char i=0;

	pCmd = str;

	for(i=0; i<NEW_CMD; i++)
	{
		if(strnicmp_t((char *)pCmd, (char *)pc_cmd_proc[i].cmd, strlen_t((char *)pc_cmd_proc[i].cmd)) == 0)
		{
			if(pc_cmd_proc[i].cmd_handler(pCmd+strlen_t((char *)pc_cmd_proc[i].cmd), out) == 1)
			{
				break;
			}
			else
			{
				strcpy_t((char *)out,"CMD_ERR\r\n");
				return 0;
			}
		}
	}

	return 1;
}

