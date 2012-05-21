#include "common.h"
#include "remotelog_if.h"

int init_rlog(unsigned int uisrvid,const char* szlocalip,const char* szsrvip,unsigned short ussrvport,int iloglv)
{
	if (g_workthread != NULL)
		return 1; //re-init
	g_iloglv = iloglv % 8;

	g_workthread = new(nothrow) workthread;
	if (g_workthread == NULL)
	{
		return 2;
	}
	g_workthread->setsrv(uisrvid,szlocalip,szsrvip,ussrvport);
	bool bret = g_workthread->start();
	if (bret == false)
	{
		if (g_workthread)
		{
			delete g_workthread;
			g_workthread = NULL;
		}
		return 3;
	}
    int icount = 5;
    while (icount-- != 0)
    {
        if (g_workthread->islogin())
        {
            break;
        }
        Sleep(200);
    }
	return 0;
}
int send_rlog(int iloglv,const char* szlog)
{
	if(g_workthread == NULL)
		return 1;
	if (!g_workthread->islogin())
		return 2;
	if(iloglv > g_iloglv)
		return 3;
	char *pbuff = NULL;
	pbuff = new(std::nothrow) char[MAX_LOG_LEN + 64];
	if (pbuff == NULL)
		return 4;
	int ilen = strlen(szlog);
	if (ilen >= MAX_LOG_LEN)
		ilen = MAX_LOG_LEN;
	pmsg_head phead = (pmsg_head)pbuff;
	phead->uicmd = CMD_LOG;
	phead->uilen = sizeof(msg_head) + sizeof(msg_log) + ilen + 1; //add 1 for '\0'
	
	pmsg_log plog = (pmsg_log)(pbuff + sizeof(msg_head));
	plog->slev = iloglv;
	plog->uithreadid = (unsigned int)GetCurrentThreadId();
	plog->uitimet = (unsigned int)time(NULL);
	plog->uiver = sizeof(msg_log);
	plog->uslen = ilen + 1;
	memcpy(plog->chbuff,szlog,ilen);
	plog->chbuff[ilen] = '\0';

	int nret = g_sendmsgqueue.PutData(pbuff,phead->uilen);
	delete [] pbuff;
	pbuff = 0;

	if (nret != 0)
		return 5;
	return 0;
}

void stop_rlog()
{
	if (g_workthread)
	{
		g_workthread->stop();

		int icount = 0;
		while (g_workthread->isrun() && icount++ < 5)
		{
#ifdef _WIN32
			Sleep(100);
#else
			usleep(100 * 1000);
#endif
		}
		//完善pthread 杀死线程待续
		delete g_workthread;
		g_workthread = NULL;
	}
}