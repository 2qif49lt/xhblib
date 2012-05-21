#ifndef OMAS_WORDKTHREAD_H_
#define OMAS_WORDKTHREAD_H_

#include "common.h"
//#include "mthread.h"
#include "msocketx.h"
#include <string>
#include <algorithm>
#include <vector>
using namespace std;


class workthread :public mthread
{

public:
	workthread()
	{
		m_SrvIP = 0;
		m_SrvPort = 0;
		m_blogin = false;
		m_chrecvbuff = NULL;
		m_remainlen = 0;
		m_bconnect = false;
	}
	~workthread(){}
public:
	void setsrv(unsigned int uisrvid,string strlocip,string strsrvip,unsigned short usssrvport);

	bool islogin(){ return m_blogin;}
public:
	virtual bool initinstance();
	virtual void mainfunction();
	virtual unsigned int exitinstance();
private:
	bool connectlogsrv();
	void sendalldata();
	bool senddata(const char* pbuff,unsigned int uilen);
	void recvalldata();
	inline void setdisconncect(){m_bconnect = false;}
	bool handle_msg(char* pmsgbuff,unsigned int uimsglen);
	void sendlogin();
	void sendkeepalive();
private:
	bool onheartbeat(const char* pdata,unsigned int uilen);
	bool onloginret(const char* pdata,unsigned int uilen);
private:
	bool					m_blogin;

	unsigned int			m_srvid;
	string					m_strsrv;			//Í¬ÏÂ ×Ö·û´®
	unsigned int			m_SrvIP;			//softcenterµØÖ·
	unsigned short			m_SrvPort;
	unsigned int			m_locip;
	msocketx				m_sock;

	char*					m_chrecvbuff;
	unsigned int			m_remainlen;
	bool					m_bconnect;
	
	static const unsigned int MAX_BUFF_LEN = 10240;
	static const unsigned int MAX_IO_COUNTER	= 40;
};

#endif	//OMAS_WORDKTHREAD_H_