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
using namespace xhb;
#include <map>
#include <vector>
using namespace std;

extern logx<mlockx_> g_log;

typedef struct tagmsgx
{
    char* pdata;
    int ilen;
    int isend;
}msgx,*pmsgx;

class sockx
{
public:
    sockx(){m_isock = -1; m_iab = 0;}
    sockx(int isock):m_isock(isock),m_iab(0){}
    ~sockx();

    bool recv();
    bool send();

    inline bool bavail()const {return m_isock >= 0;}
    void attach(int isock){m_isock = isock;m_iab = 0;}
    void detach();

    ptrqueuex<msgx,10,false> m_rsq;
    int m_iab;
    int m_isock;
};

class epollx
{
public:
    epollx();
    ~epollx();

public:
    bool start(char* szip,unsigned short usport,int icapacity);
    void stop();
public:
    void addsock(int isock);
    void delsock(int isock);
    void dispatch(int isock);
    bool readx(int isock);
    bool writex(int isock);
    bool bfull(){return m_socknum >= m_capacity;}
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