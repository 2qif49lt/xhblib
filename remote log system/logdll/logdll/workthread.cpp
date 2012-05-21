#include "workthread.h"
#include "common.h"

#include <string>
using namespace std;
void workthread::setsrv(unsigned int uisrvid,string strlocip,string strsrvip,unsigned short usssrvport)
{
	m_strsrv = strsrvip;
	m_SrvIP = getipbystring(strsrvip);
	m_locip = getipbystring(strlocip);
	m_SrvPort = usssrvport;
	m_srvid = uisrvid;
}

bool workthread::initinstance()
{
	if (m_chrecvbuff == NULL)
	{
		m_chrecvbuff = new(nothrow) char[MAX_BUFF_LEN];
		if (m_chrecvbuff == NULL)
			return false;
	}

	m_remainlen = 0;

	bool bret = msocketx::startup();
	if (bret == false)
		return false;

	return mthread::initinstance();
}
unsigned int workthread::exitinstance()
{
	m_sock.close();

	if (m_chrecvbuff != NULL)
	{
		delete [] m_chrecvbuff;
		m_chrecvbuff = NULL;
	}
	msocketx::clearup();
	return mthread::exitinstance();
}
bool workthread::connectlogsrv()
{
	if (true == m_bconnect)
		return true;

	m_sock.close();
	int nret = m_sock.connect(m_strsrv.c_str(),m_SrvPort,true);
	if (nret == SOCKET_ERROR)
		return false;
	else
	{
		m_sock.setblock(false);
		m_sock.setsockbuff();
		return true;
	}
}

void workthread::mainfunction()
{
	static unsigned int uilastconntick = 0;
	while(isrun())
	{
		if (false == m_bconnect)
		{
			if (getinterval(uilastconntick,getticktime()) >= 30 * 1000)
			{
				m_bconnect = connectlogsrv();
				if (m_bconnect == true)
					sendlogin();
				uilastconntick = getticktime();
			}
			millisleep(100);
		}
		else
		{
			sendkeepalive();
			sendalldata();
			recvalldata();
		}
	}
}
void workthread::sendalldata()
{
	unsigned int uimillisec = 5;
	int nret = 0;
	unsigned int uicount = 0;
	char* pbuff = NULL;
	unsigned int uilen = 0;
	bool bret = false;

	nret = m_sock.wait(uimillisec,CAN_WRITEX);

	if (nret > 0 && ((nret & CAN_WRITEX) == CAN_WRITEX))
	{
		uicount = 0;
		while (uicount++ < MAX_IO_COUNTER)
		{
			if (g_sendmsgqueue.GetData(pbuff,uilen) == 0)
			{
				bret = senddata(pbuff,uilen);
				g_sendmsgqueue.ReleaseBuff(pbuff);

				if (bret == false)
				{
					m_sock.close();
					m_bconnect = false;
					m_blogin = false;
					return;
				}
			}
			else
				break;
		}
	}
}
bool workthread::senddata(const char* pbuff,unsigned int uilen)
{
	if (pbuff == NULL || uilen == 0)
		return true;
 
	unsigned int uisended = 0;

	while (uisended < uilen)
	{
		int nret = m_sock.send(pbuff + uisended ,uilen - uisended,0);
		if (nret != SOCKET_ERROR && nret > 0)
		{
			uisended += nret;
		}
		else
		{
			int nerr = msocketx::getlasterror();
#ifdef _WIN32
			if (nerr != WSAEWOULDBLOCK)
#else
			if (nerr != EWOULDBLOCK)
#endif
			{
				return false;
			}
		}
	}
	return true;
}
void workthread::recvalldata()
{
	unsigned int uimillisec = 5;
	int nret = 0;
	char* pbuff = NULL;
	unsigned int uilen = 0;
	bool bret = false;

	if ((nret = m_sock.wait(uimillisec,CAN_READX) > 0) && ((nret & CAN_READX) == CAN_READX))
	{
		int nmaxrecvlen = MAX_BUFF_LEN - m_remainlen;
		nret  = m_sock.recv(m_chrecvbuff + m_remainlen,nmaxrecvlen,0);

		if (nret != SOCKET_ERROR && nret > 0)
		{
			m_remainlen += nret;
			pmsg_head phead = NULL;
			char* ptmpbuff = m_chrecvbuff;
			while (m_remainlen >= sizeof(msg_head))
			{
				phead = (pmsg_head)ptmpbuff;
				if (phead->uilen > m_remainlen)
					break;

				bret = handle_msg(ptmpbuff,phead->uilen);
				if (bret == false)
				{
					m_sock.close();
					m_bconnect = false;
					m_blogin = false;
					return;
				}
				ptmpbuff += phead->uilen;
				m_remainlen -= phead->uilen;
			}
			if (m_remainlen > 0 && ptmpbuff != m_chrecvbuff)
				memmove(m_chrecvbuff,ptmpbuff,m_remainlen);
		}
		else
		{
			int nerr = msocketx::getlasterror();
#ifdef _WIN32
			if (nerr != WSAEWOULDBLOCK)
#else
			if (nerr != EWOULDBLOCK)
#endif
			{
				m_sock.close();
				m_bconnect = false;
				m_blogin = false;
				return;
			}
		}

	}
}

bool workthread::handle_msg(char* pmsgbuff,unsigned int uimsglen)
{
	pmsg_head phead = (pmsg_head)pmsgbuff;

	switch (phead->uicmd)
	{
	case CMD_HEARTBEAT:
		return onheartbeat(pmsgbuff,uimsglen);
// 	case AgentDll_Cmd_LogOnRet:
// 		return onloginret(pmsgbuff,uimsglen);
	default:
		return false;
	}
}
void workthread::sendlogin()
{
	char chbuff[256] = {0};

	pmsg_head phead = (pmsg_head)chbuff;
	phead->uicmd = CMD_LOGIN;
	phead->uilen = sizeof(msg_head) + sizeof(msg_login);

	pmsg_login plogin = (pmsg_login)(chbuff + sizeof(msg_head));
	plogin->uiver = sizeof(msg_login);
	plogin->uiida = m_srvid;
	plogin->uiip = m_locip;

	bool bsend = senddata(chbuff,phead->uilen);
	m_blogin = bsend;
}
void workthread::sendkeepalive()
{
	static unsigned int uilastkeepalive = getticktime();
	if (getinterval(uilastkeepalive,getticktime()) > 60 * 1000)
	{
		char chbuff[256] = {0};
		pmsg_head phead = (pmsg_head)chbuff;
		phead->uicmd = CMD_HEARTBEAT;
		phead->uilen = sizeof(msg_head) + sizeof(msg_heartbeat);
		pmsg_heartbeat pbeat = (pmsg_heartbeat)(chbuff + sizeof(msg_head));
		pbeat->uiver = sizeof(msg_heartbeat);
		senddata(chbuff,phead->uilen);
		uilastkeepalive = getticktime();
	}
}
bool workthread::onheartbeat(const char* pdata,unsigned int uilen)
{
	return true;
}
bool workthread::onloginret(const char* pdata,unsigned int uilen)
{
	m_blogin = true;
	return true;
}