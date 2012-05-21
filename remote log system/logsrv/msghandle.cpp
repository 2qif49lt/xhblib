#include "epollx.h"
#include "msg.h"
bool sockx::handle_login(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock)
{
    pmsg_head phead = (pmsg_head)pbuff;
    pmsg_login pbody = (pmsg_login)(pbuff + sizeof(msg_head));
    if (sizeof(msg_login) != pbody->uiver)
        return false;   //different version login,no support now.
	beat();
	m_localip = pbody->uiip;
	in_addr iip,lip;
	char sziip[32],szlip[32];
	iip.s_addr = m_uiip;
	lip.s_addr = m_localip;
	strcpy(sziip,inet_ntoa(iip));
	strcpy(szlip,inet_ntoa(lip));
	g_log.log(LOG_LV_DEBUG,"%s:%u login localip:%s",sziip,m_usport,szlip);
	return m_log.init(pbody->uiida,sziip,szlip);
}

bool sockx::handle_log(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock)
{
    pmsg_head phead = (pmsg_head)pbuff;
    pmsg_log pbody = (pmsg_log)(pbuff + sizeof(msg_head));
    if (sizeof(msg_log) != pbody->uiver)
        return false;   //different version login,no support now.
	beat();
	pbody->chbuff[pbody->uslen - 1] = '\0';

    return m_log.log(pbody->uitimet,pbody->slev,pbody->uithreadid,pbody->chbuff);
}

bool sockx::handle_heartbeat(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock)
{
    pmsg_head phead = (pmsg_head)pbuff;
    pmsg_heartbeat pbody = (pmsg_heartbeat)(pbuff + sizeof(msg_head));
    if (sizeof(msg_heartbeat) != pbody->uiver)
        return false;   //different version login,no support now.
	beat();
    return sendmessage(pbuff,uilen);
}

bool sockx::sendmessage(char* pdata,unsigned uilen)
{
    if(m_sq.isfull())
        return false;
    pmsgx pm = (pmsgx)alloc(sizeof(msgx));
    if(pm == NULL)
        return false;
    char * buff = alloc(uilen);
    if(buff == NULL)
    {
        release((char*)pm);
        return false;
    }
    memcpy(buff,pdata,uilen);
    pm->pdata = buff;
    pm->ilen = uilen;
    pm->isend = 0;
    m_sq.push_back(pm);
    return true;
}
