#include "includes.h"

static msg_info MsgGsm;
static msg_flash_info g_Gsmflash;
static gps_block_t GsmBlindInPost;//ä�����α�
static gps_block_t GsmBlindOutPost;//ä�����α�

//���еĳ�ʼ��
void MsgGsm_Init(void)
{
	MsgGsm.In = MsgGsm.Out = 0;
	memset_t((char *)MsgGsm.Buf,0,sizeof(MsgGsm.Buf));

    MsgGsm_BlindIndex_read();
    MsgGsm_BlindOutdex_read();
}
//���FLASH
void MsgGsm_Clear(void)
{
	LOG("[msg] clear\r\n");
	MsgGsm.In = MsgGsm.Out = 0;
	g_Gsmflash.flashin = g_Gsmflash.flashout = 0;
	MsgGsm_BlindPost_clear();
    MsgGsm_BlindIndex_clear();
    MsgGsm_BlindOutdex_clear();
}
//����ä�����ݣ�һ�α���n��
void MsgGsm_SaveBlindData(unsigned int index,unsigned char *buf)
{
	unsigned int page=0;

	if ((index*WRITE_BLIND_SIZE) > (PAGE_GSMBLIND_END-PAGE_GSMBLIND_START+4096))
	{
		return;
	}

	page = PAGE_GSMBLIND_START+(index*WRITE_BLIND_SIZE);
	flash_blinddata_write(page,WRITE_BLIND_SIZE,buf);
}
//��ȡä������ һ�ζ�ȡһ��
void MsgGsm_ReadBlindData(unsigned int index,unsigned char *buf)
{
    unsigned int page=0;

    if ((index*READ_BLIND_SIZE) > (PAGE_GSMBLIND_END-PAGE_GSMBLIND_START+4096))
    {
	   	return;
    }

	page = PAGE_GSMBLIND_START+(index*READ_BLIND_SIZE);
	flash_blinddata_read(page,READ_BLIND_SIZE,buf); 
}

//�����
unsigned char MsgGsm_InputMsg(unsigned char *msg,unsigned short len)
{
	if ((msg==NULL) || (len==0) || (len>DATALEN))
    {
        return 0;
    }
	if ((MsgGsm.In >= MSGCOUNT) || (MsgGsm.Out >= MSGCOUNT))// �ڴ���ִ���
	{
		MsgGsm.In = MsgGsm.Out = 0;
		LOG("[msg] input failed\r\n");

		return 0;
	}
		
	memset_t((char *)MsgGsm.Buf[MsgGsm.In].data,0,sizeof(MsgGsm.Buf[MsgGsm.In].data));
	memcpy_t(MsgGsm.Buf[MsgGsm.In].data,msg,len);
	MsgGsm.Buf[MsgGsm.In].len = (unsigned char)len;
	
	if (++MsgGsm.In >=  MSGCOUNT)
	{
	    MsgGsm.In = 0;
	}

	if (MsgGsm.In == MsgGsm.Out)//�ڴ������˱���FLASH
	{
		MsgGsm_SaveFlash((unsigned char *)MsgGsm.Buf);
	}

	return 1;
}

//������
unsigned char MsgGsm_OutputMsg(unsigned char *msg)
{
	unsigned char OutMsg[DATALEN+1]={0};
	unsigned char ret=0;
	unsigned short len=0;

	if ((MsgGsm.In >= MSGCOUNT) || (MsgGsm.Out >= MSGCOUNT))// �ڴ���ִ���
    {
		MsgGsm.In = MsgGsm.Out = 0;
		LOG("[msg] output failed\r\n");

		return 0;
    }

#if (MSG_ACCORD_TO_TIME_SEND == 1)
	(void)ret;

	if (MsgGsm_ReadFlash((unsigned char *)OutMsg))//�ȶ�ȡflash�е�����
	{
		if (OutMsg[0] > DATALEN)//���ִ���
		{
			LOG("[msg] flash len err %d,%x,%x\r\n",OutMsg[0],OutMsg[1],OutMsg[2]);
			MsgGsm_Clear();

			return 0;
		}
		memcpy_t(msg,&OutMsg[1],OutMsg[0]);
		len = OutMsg[0];
	}
	else//���ڴ�
	{
		if (MsgGsm.In != MsgGsm.Out)
	    {
	        if (MsgGsm.Buf[MsgGsm.Out].len > DATALEN)//���ִ���
	        {
				MsgGsm.In = MsgGsm.Out = 0;
				LOG("[msg] ram len err %d\r\n",MsgGsm.Buf[MsgGsm.Out].len);

				return 0;
	        }
	        memcpy_t(msg,MsgGsm.Buf[MsgGsm.Out].data,MsgGsm.Buf[MsgGsm.Out].len);
	        len = MsgGsm.Buf[MsgGsm.Out].len;

			if (++MsgGsm.Out >= MSGCOUNT)
			{
				MsgGsm.Out = 0;
			}
	    }
		else
		{
			return 0;
		}
	}
#else
    if (MsgGsm.In != MsgGsm.Out)//���ڴ�
    {
        if (MsgGsm.Buf[MsgGsm.Out].len > DATALEN)//���ִ���
        {
			MsgGsm.In = MsgGsm.Out = 0;
			LOG("[msg] ram len err %d\r\n",MsgGsm.Buf[MsgGsm.Out].len);

			return 0;
        }
        memcpy_t(msg,MsgGsm.Buf[MsgGsm.Out].data,MsgGsm.Buf[MsgGsm.Out].len);
        len = MsgGsm.Buf[MsgGsm.Out].len;

		if (++MsgGsm.Out >= MSGCOUNT)
		{
			MsgGsm.Out = 0;
		}
    }
    else//�ڴ���û�����ݾͶ�FLASH
    {
		ret = MsgGsm_ReadFlash((unsigned char *)OutMsg); 
		                      
		if (ret == 0)//FLASH��û������
		{
			return 0;
		}
	    else
		{		
			if (OutMsg[0] > DATALEN)//���ִ���
			{
				LOG("[msg] flash len err %d,%x,%x\r\n",OutMsg[0],OutMsg[1],OutMsg[2]);
				MsgGsm_Clear();

				return 0;
			}
			memcpy_t(msg,&OutMsg[1],OutMsg[0]);
			len = OutMsg[0];
		}
	}
#endif
	
	return len;	
}
//����FLASH  (ä������)
unsigned char MsgGsm_SaveFlash(unsigned char *buf)
{
	msg_flash_info *info = &g_Gsmflash;

	if(gc_debug == 1)//����ģʽ
	{
		return 0;
	}
	if((info->flashin>FLASHMAXCNT) || (info->flashin%MSGCOUNT!=0))// FLASH���ִ���        
	{   
		LOG("[msg] flash save err in[%d]\r\n",info->flashin);
		goto fail;
	}
	
	if(info->flashout>FLASHMAXCNT)           
	{
		LOG("[msg] flash save err out[%d]\r\n",info->flashout);
		goto fail;
	}

	MsgGsm_SaveBlindData(info->flashin/MSGCOUNT,buf);

	info->flashin += MSGCOUNT;
	if(info->flashin >= FLASHMAXCNT)
	{
		info->flashin = 0;
	}
	//��������ҳΪ��λ�������α��ƶ�Ҳ����ҳΪ��λ
	if(info->flashout >= info->flashin)
	{
		if((info->flashout - info->flashin < PAGECOUNT)&&(info->flashin%PAGECOUNT == 0))
		{
			info->flashout += (PAGECOUNT - info->flashout%PAGECOUNT);
			if(info->flashout >= FLASHMAXCNT)
	        {
				info->flashout = 0;
	        }
	        MsgGsm_BlindOutdex_save(info->flashout);
		}
	}
    MsgGsm_BlindIndex_save(info->flashin);

    LOG("[msg] gsm save: %d  %d\r\n",info->flashin,info->flashout);
    
    return 1;

fail:
	MsgGsm_Clear();
	return 0;
}
//��ȡFLASH   
unsigned char MsgGsm_ReadFlash(unsigned char *buf)
{	
	msg_flash_info *info = &g_Gsmflash;
	
	if(info->flashin > FLASHMAXCNT)// FLASH���ִ���        
    {
		LOG("[msg] flash read err in[%d]\r\n",info->flashin);
		goto fail;
    }

	if(info->flashout > FLASHMAXCNT)           
	{
		LOG("[msg] flash read err out[%d]\r\n",info->flashout);
		goto fail;
	}

	if(info->flashin == info->flashout)
	{
		return 0;
	}

	MsgGsm_ReadBlindData(info->flashout,buf);  
    
    info->flashout ++;
    if(info->flashout >= FLASHMAXCNT)
    {
        info->flashout = 0;
    }
    MsgGsm_BlindOutdex_save(info->flashout);

    LOG("[msg] gsm read: %d  %d\r\n",info->flashin,info->flashout);
	
	return 1;	
	
fail:
	MsgGsm_Clear();
	return 0;	
}

/*ä���α꣬���������أ������򷵻أ�*/
int Msg_PostAdd(gps_block_t *pRecord)
{
	int ret=0;
	
	pRecord->offset += 4;
	if(pRecord->offset > (FLASH_BLIND_SIZE-1))
	{
		ret = 1;
	}
	return ret;
}

//���
void MsgGsm_BlindPost_clear(void)
{
    GsmBlindInPost.offset = 0;
	GsmBlindInPost.sector = PAGE_IN_GSMBLIND_START;

    GsmBlindOutPost.offset = 0;
	GsmBlindOutPost.sector = PAGE_OUT_GSMBLIND_START;
}
void MsgGsm_BlindIndex_clear(void)
{
	flash_blindInIndex_erase(PAGE_IN_GSMBLIND_START, PAGE_IN_GSMBLIND_END);
}
void MsgGsm_BlindOutdex_clear(void)
{
	flash_blindOutIndex_erase(PAGE_OUT_GSMBLIND_START, PAGE_OUT_GSMBLIND_END);
}

//���淽ʽΪѭ���ƶ�����
void MsgGsm_BlindIndex_save(unsigned int indexPost)
{
	unsigned int now_index=0;
	unsigned char buf[4]={0};

    now_index = indexPost;
    uint32_to_uint8(now_index,buf);

    if(GsmBlindInPost.offset > (FLASH_BLIND_SIZE-1))
	{		
		MsgGsm_BlindIndex_clear();
		GsmBlindInPost.offset = 0;
	}

//	LOG("GPS write blind offset:%d\r\n",GsmBlindInPost.offset);
	
	flash_blindIndex_write(GsmBlindInPost.sector+GsmBlindInPost.offset,4,buf);

	GsmBlindInPost.offset += 4;	
}
void MsgGsm_BlindIndex_read(void)
{
    unsigned char buf[4]={0};
	unsigned int now_index=0;
	unsigned int last_index=0;
	int sign=0;

	/*�ҳ�flash��д���¼�¼��λ��*/
	GsmBlindInPost.offset = 0;
	GsmBlindInPost.sector = PAGE_IN_GSMBLIND_START;
	while(1)
	{
		buf[0] = buf[1] = buf[2] = buf[3] = 0;
        flash_blindIndex_read(GsmBlindInPost.sector+GsmBlindInPost.offset,4,buf);
		
		now_index = uint8_to_uint32(buf);
		if(now_index == 0xFFFFFFFF)
		{
			break;
		}
		if(1 == Msg_PostAdd(&GsmBlindInPost))
		{
			sign = 1;
            last_index = now_index;
			break;
		}
		last_index = now_index;
	}

    if(1 == sign)
	{
		GsmBlindInPost.offset = 0;
		GsmBlindInPost.sector = PAGE_IN_GSMBLIND_START;
		MsgGsm_BlindIndex_clear();
	}
    
    //LOG("GPS in blind:%d , offset:%d\r\n",last_index,GsmBlindInPost.offset);

	g_Gsmflash.flashin = last_index;
}

//���淽ʽΪѭ���ƶ�����
void MsgGsm_BlindOutdex_save(unsigned int outdexPost)
{
	unsigned int now_index=0;
	unsigned char buf[4]={0};

    now_index = outdexPost;
    uint32_to_uint8(now_index,buf);

    if(GsmBlindOutPost.offset > (FLASH_BLIND_SIZE-1))
	{		
		MsgGsm_BlindOutdex_clear();
		GsmBlindOutPost.offset = 0;
	}

//	LOG("GPS write blind offset:%d\r\n",GsmBlindInPost.offset);
	
	flash_blindIndex_write(GsmBlindOutPost.sector+GsmBlindOutPost.offset,4,buf);

	GsmBlindOutPost.offset += 4;	
}
void MsgGsm_BlindOutdex_read(void)
{
    unsigned char buf[4]={0};
	unsigned int now_outdex=0;
	unsigned int last_outdex=0;
	int sign=0;

	/*�ҳ�flash��д���¼�¼��λ��*/
	GsmBlindOutPost.offset = 0;
	GsmBlindOutPost.sector = PAGE_OUT_GSMBLIND_START;
	while(1)
	{
		buf[0] = buf[1] = buf[2]= buf[3] = 0;
        flash_blindIndex_read(GsmBlindOutPost.sector+GsmBlindOutPost.offset,4,buf);//every time read 2bytes
		
		now_outdex = uint8_to_uint32(buf);
		if(now_outdex == 0xFFFFFFFF)
		{
			break;
		}
		if(1 == Msg_PostAdd(&GsmBlindOutPost))
		{
			sign = 1;
            last_outdex = now_outdex;
			break;
		}
		last_outdex = now_outdex;
	}
	
	if(1 == sign)
	{
		GsmBlindOutPost.offset = 0;
		GsmBlindOutPost.sector = PAGE_OUT_GSMBLIND_START;
		MsgGsm_BlindOutdex_clear();
	}

    //LOG("GPS out blind:%d , offset:%d\r\n",last_outdex,GsmBlindOutPost.offset);

	g_Gsmflash.flashout = last_outdex;
}

unsigned int MsgGsm_GetFlashIn(void)
{
	return g_Gsmflash.flashin;
}

unsigned int MsgGsm_GetFlashOut(void)
{
	return g_Gsmflash.flashout;
}
