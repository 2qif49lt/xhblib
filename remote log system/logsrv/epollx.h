#ifndef EPOLLX_LINUX_XHB_H_
#define EPOLLX_LINUX_XHB_H_
#include <errno.h>
#include <stdio.h>
#include <unistd.h> //read,write,close sleep fork NULL 
#include <sys/types.h>  //pid,uid,etc.
#include <sys/syscall.h> //for syscall gettid
#include <string.h> //for string function,memset,bzero.
#include <netinet/in.h> //for  struct in_addr
#include <sys/socket.h>
#include <arpa/inet.h>  //for htonl inet_ntoa etc
#include <fcntl.h>  //file control options for  fcntl open create etc
#include <sys/epoll.h>  //epoll
#include <sys/time.h> //for timeval,gettimeofday,select

#include <new>
#include "mthread.h"
#include "ptrqueuex.h"
#include "logx.h"
#include "memoryx.h"
using namespace xhb;
#include <map>
#include <vector>
using namespace std;

extern logx<mlockx_> g_log;
extern bitpoolx_<mlockx_> g_mem;

extern char* alloc(const unsigned int n);
extern void release(char* pmem);

#include "msg.h"
#include "logmgr.h"

extern unsigned int getinterval(unsigned int uilast,unsigned int uinow);
extern unsigned int getticktime();

typedef struct tagmsgx
{
    char* pdata;
    int ilen;
    int isend;
}msgx,*pmsgx;

const unsigned int MAXMSGSIZE = 10240;
const unsigned int MAXDIETIME = 120*1000;   //millisecond

class sockx
{
public:
    // all operation would be done in one thread and only. so do not need to consider synchronization
    sockx():m_isock(-1),m_uialive(0),m_uihr(0),m_uiip(0),m_usport(0)
    {
        m_pdata = alloc(MAXMSGSIZE);
    }
    sockx(int isock,unsigned int uiip,unsigned short usport):m_isock(isock),m_uihr(0),m_uiip(uiip),m_usport(usport)
    {
        m_uialive = getticktime();
        m_pdata = alloc(MAXMSGSIZE);
    }
    ~sockx();

    bool recv();
    bool send();
    bool sendmessage(char* pdata,unsigned int uilen);//send a 'complete' 'valid' message
    
    inline bool bavail()const 
    {
        return m_isock >= 0;
    }
	inline bool balive()const
	{
		 return getinterval(m_uialive,getticktime()) <= MAXDIETIME;
	}
    inline void attach(int isock,unsigned int uiip,unsigned short usport)
    {
        m_isock = isock; 
        m_uiip = uiip;
        m_usport = usport;
        m_uihr = 0; 
        m_uialive = getticktime();
    }
    void detach();

    int m_isock;
    unsigned int m_uialive;

    unsigned int m_uihr;
    char* m_pdata;
    
    ptrqueuex<msgx,5,false> m_sq; // send queue

    unsigned int m_uiip;	//internet ip
	unsigned int m_localip;	//protocal login's ip,local ip;
    unsigned short m_usport;

    logmgr<> m_log;

    bool domsg(unsigned uicmd,unsigned int uilen,char* pbuff);
    bool handle_login(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock);
    bool handle_log(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock);
    bool handle_heartbeat(char* pbuff,unsigned int uilen,unsigned int uiip,unsigned short usport,int isock);


	inline void beat() {m_uialive = getticktime();}
};

class epollx
{
public:
    epollx();
    ~epollx();

public:
	//if you do lots of operators like open file pipe etc you should give a bigger capacity paramter
    bool start(char* szip,unsigned short usport,int icapacity);
    void stop();
public:
    void addsock(int isock);
    void delsock(int isock);
    void dispatch(int isock,unsigned int uiip,unsigned short usport);
    bool readx(int isock);
    bool writex(int isock);
    bool bfull(int isock){return (isock <= m_capacity) && (m_socknum >= m_capacity);}
	void checkalive();
private:
    bool createepfd(); 
    bool createlistener(char* szip,unsigned short usport); 
    bool createthread();
    int getcpunum();
private:
    volatile int m_socknum;  //all socket num.
    vector<sockx> m_sockvec;    //used and unused socket.
    vector<mthread*> m_threadvec; //all thread , n cpus io, 1 lister.
    vector<int> m_epfdvce;    //epoll fd.
    int m_capacity;
    int m_fdlister;
};

class epioer : public mthread
{
public:
    epioer(){m_epfd = -1;m_socknum = 0;m_pep = NULL;}
    ~epioer(){}
public:
 //   virtual bool initinstance();
    virtual void mainfunction();
 //   virtual unsigned int exitinstance();
   
    void setepollx(epollx* pep){m_pep = pep;}
    void setepfd(int epfd){m_epfd = epfd;}
    
    void addsock(int isock);
    void delsock(int isock);

    bool setctl(int isock,int ievent);
  
private:
 

private:
    int m_epfd;
    volatile int m_socknum;  //this io thread socket num epoll add.
    epollx* m_pep;
};

class eplister : public mthread
{
public:
    eplister(){m_pep = NULL; m_listerfd = -1;}
public:
//    virtual bool initinstance();
    virtual void mainfunction();
 //   virtual unsigned int exitinstance();

    void setepollx(epollx* pep){m_pep = pep;}
    void setlister(int ilfd){m_listerfd = ilfd;}
private:
    epollx* m_pep;
    int m_listerfd;
};

#endif //EPOLLX_LINUX_XHB_H_