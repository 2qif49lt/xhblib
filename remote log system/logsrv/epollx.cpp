#include "epollx.h"


logx<mlockx_> g_log;
bitpoolx_<mlockx_> g_mem;

char* alloc(const unsigned int n)
{
    if(n > 0)
        return g_mem.alloc(n);
}
void release(char* pmem)
{
    g_mem.free(pmem);
}

bool setnoblock(int isock)
{
    int opts = fcntl(isock,F_GETFL);
    if (opts < 0)
        return false;

    opts |= O_NONBLOCK;

    return fcntl(isock,F_SETFL,opts) != -1;
}
unsigned int getticktime()
{
    unsigned int currentTime;
#ifdef _WIN32
    currentTime = GetTickCount();
#else
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec/1000;
#endif
    return currentTime;
}

unsigned int getinterval(unsigned int uilast,unsigned int uinow)
{
    if( uinow < uilast )
        return 0xFFFFFFFF - uilast + uinow;
    else
        return uinow - uilast;
}

bool sockx::recv()
{
    while(true)
    {
        int ipr = 0,inr = MAXMSGSIZE - m_uihr;
     
        ipr = read(m_isock,m_pdata + m_uihr,inr);
        if (ipr == 0)
        { 
            g_log.log(LOG_LV_DEBUG,"read ret 0,errno:%d.",errno);
            return false;
        }
        if (ipr == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                g_log.log(LOG_LV_DEBUG,"read ret -1 and errno != EAGAIN,errno:%d.",errno);
                return false;
            }
        }
        else
        {
            m_uihr += ipr;

            char* pdate = m_pdata;
            while (m_uihr >= sizeof(msg_head)) 
            {
                pmsg_head phead = (pmsg_head)pdate;
                if(phead->uilen < sizeof(msg_head))
                {
                    for (int i = 0; i != m_uihr;++i)
                    {
                        if (i%4 == 0)
                        {
                            printf("\n");
                        }
                        printf("%02x ",m_pdata[i]);
                       
                    }
                    return false;
                }
                if (m_uihr >= phead->uilen) //atleast one complete message
                {

                    if(!domsg(phead->uicmd,phead->uilen,pdate))
                        return false;
                    pdate += phead->uilen;
                    m_uihr -= phead->uilen;
                }
                else
                    break;  //incomplete message

            }
            if (pdate != m_pdata && m_uihr > 0)
                memmove(m_pdata,pdate,m_uihr);

            if(ipr < inr)
                break;
        }
        
    }
    return true;
}

bool sockx::send()
{
 //   g_log.log(LOG_LV_DEBUG,"sockid %d send.",m_isock);
    pmsgx pm = NULL; 
    while(pm = m_sq.front())
    {
        char *buff = pm->pdata;
        int ip = 0;
        while (pm->isend != pm->ilen)
        {
            ip = write(m_isock,buff + pm->isend,pm->ilen - pm->isend);
            if (ip == 0)
            {
                g_log.log(LOG_LV_DEBUG,"write ret 0,errno:%d.",errno);
                return false;
            }
            if (ip == -1)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    return true;
                else
                {
                    g_log.log(LOG_LV_DEBUG,"write ret -1 and errno != EAGAIN,errno:%d.",errno);
                    return false;
                }
            }
            else
            {
                pm->isend += ip;
            }
        }
        m_sq.pop_front();
        release((char*)pm);
        release(buff);
    }
    return true;
}
bool sockx::domsg(unsigned uicmd,unsigned int uilen,char* pbuff)
{
    switch (uicmd)
    {
    case CMD_LOGIN:
        return handle_login(pbuff,uilen,m_uiip,m_usport,m_isock);
    	break;
    case CMD_LOG:
        return handle_log(pbuff,uilen,m_uiip,m_usport,m_isock);
        break;
    case CMD_HEARTBEAT:
        return handle_heartbeat(pbuff,uilen,m_uiip,m_usport,m_isock);
        break;
    default:
        break;
    }
    return true;
}
void sockx::detach()
{
    m_isock = -1;
    m_uiip = 0;
    m_usport = 0;
    m_uialive = 0;
    m_uihr = 0;
	
    pmsgx pm = NULL;
    while(pm = m_sq.pop_front())
    {
        release(pm->pdata);
        release((char*)pm);
    }
	m_log.closelog();
}

sockx::~sockx()
{
    if(m_isock > 0)
        close(m_isock);
    detach();
    release(m_pdata);
}

epollx::epollx()
{
    m_socknum = 0;
    m_capacity = 0;
    m_fdlister = -1;
}
epollx::~epollx()
{
    for (int i = 0; i != m_epfdvce.size(); ++i)
    {
        if(m_epfdvce[i] >= 0)
            close(m_epfdvce[i]);
    }
    if(m_fdlister >= 0)
        close(m_fdlister);

    for (int i = 0; i != m_threadvec.size(); ++i)
    {
        if (m_threadvec[i] != NULL)
        {
            if (m_threadvec[i]->isrun())
            {
                m_threadvec[i]->stop();
                usleep(200 * 1000);
            }
            delete m_threadvec[i];
        }
    }
}

void epollx::stop()
{
    for (int i = 0; i != m_threadvec.size(); ++i)
    {
        if (m_threadvec[i] != NULL)
        {
            if (m_threadvec[i]->isrun())
                m_threadvec[i]->stop();
        }
    }
}
bool epollx::createlistener(char* szip,unsigned short usport)
{
    m_fdlister = socket(AF_INET,SOCK_STREAM,0);
    if (m_fdlister == -1)
        return false;

    setnoblock(m_fdlister);

    sockaddr_in myaddr = {0};
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(usport);
    if(szip == NULL)
        myaddr.sin_addr.s_addr = INADDR_ANY;
    else
        myaddr.sin_addr.s_addr = inet_addr(szip);

    if (bind(m_fdlister,(sockaddr*)&myaddr,sizeof(myaddr)) == -1)
        return false;

    return listen(m_fdlister,50) != -1;
}
int epollx::getcpunum()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}
void epollx::checkalive()
{
	for (int i = 0; i != m_sockvec.size(); ++i)
	{
		if (m_sockvec[i].bavail())
		{
			if (!m_sockvec[i].balive())
			{
				close(i);
			}
		}
	}
}
bool epollx::createepfd()
{
    int icpu = getcpunum();
    for (int i = 0; i != icpu; ++i)
    {
        int itmp = epoll_create(50);
        if(itmp == -1)
        {
            g_log.log(LOG_LV_ERROR,"epoll_create RET -1,errno:%d",errno);
            return false;
        }
        m_epfdvce.push_back(itmp);
    }
    return true;
}
bool epollx::createthread()
{
    for (int i = 0; i != m_epfdvce.size(); ++ i)
    {
        epioer* pioer = new(nothrow) epioer;
        if(pioer == NULL)
            return false;
        pioer->setepfd(m_epfdvce[i]);
        pioer->setepollx(this);
        if(!pioer->start())
            return false;
        m_threadvec.push_back(pioer);
    }
    eplister* plister = new(nothrow) eplister;
    if(plister == NULL)
        return false;
    plister->setepollx(this);
    plister->setlister(m_fdlister);
    if(!plister->start())
        return false;
    m_threadvec.push_back(plister);
    return true;
}
bool epollx::start(char* szip,unsigned short usport,int icapacity)
{
    m_capacity = icapacity;
    m_sockvec.resize(icapacity + getcpunum() + 10);
    if(!createlistener(szip,usport))
        return false;
    if(!createepfd())
    {   
        g_log.log(LOG_LV_ERROR,"createfd fail!");
        return false;
    }
    if(!createthread())
    {
        g_log.log(LOG_LV_ERROR,"createthread fail!");
        return false;
    }
    g_log.log(LOG_LV_INFO,"start succ!");
    return true;
}
void epioer::mainfunction()
{
    g_log.log(LOG_LV_INFO,"epioer thread come!");
    const int maxevsize = 4096;

    int ifd = 0,nfd= 0,i = 0;

    epoll_event evarr[maxevsize];

    int itimeout = 50;
    while (isrun())
    {
        int iwaitnum = (m_socknum > 0 ? std::min((int)m_socknum,maxevsize) : maxevsize);
        ifd = epoll_wait(m_epfd,evarr,iwaitnum,itimeout);  
        if (ifd == -1)
        {
            if(errno == EINTR)
                continue;
            g_log.log(LOG_LV_DEBUG,"epoll_wait fail,errno %d,socknum %d.",errno,iwaitnum);
            return;
        }
        for (i = 0; i != ifd; ++i)
        {
            if (evarr[i].events & EPOLLERR || evarr[i].events & EPOLLHUP)
            {
                g_log.log(LOG_LV_DEBUG,"fd %d event err %d.",evarr[i].data.fd,evarr[i].events);
                delsock(evarr[i].data.fd);
            }else if (evarr[i].events & EPOLLIN)
            {
                if (!m_pep->readx(evarr[i].data.fd))
                {
                    g_log.log(LOG_LV_DEBUG,"sock %d read false.",evarr[i].data.fd);
                    delsock(evarr[i].data.fd);
                }
                else
                    setctl(evarr[i].data.fd,EPOLLOUT);        

            }else if (evarr[i].events & EPOLLOUT)
            {
                if (!m_pep->writex(evarr[i].data.fd))
                {
                    g_log.log(LOG_LV_DEBUG,"sock %d write false.",evarr[i].data.fd);
                    delsock(evarr[i].data.fd);
                }
                else
                    setctl(evarr[i].data.fd,EPOLLIN);
            }
        }
    }
    g_log.log(LOG_LV_INFO,"epioer thread exit!");
}
bool epioer::setctl(int isock,int ievent)
{
    epoll_event ev;
    ev.data.fd = isock;
    ev.events = ievent | EPOLLET;			
    return epoll_ctl(m_epfd, EPOLL_CTL_MOD, isock, &ev) == 0;
}
void epioer::delsock(int isock)
{
    __sync_sub_and_fetch(&m_socknum,1);
    epoll_ctl(m_epfd,EPOLL_CTL_DEL,isock,NULL);
    close(isock);
    m_pep->delsock(isock);
}
void epollx::delsock(int isock)
{
    __sync_sub_and_fetch(&m_socknum,1);
    m_sockvec[isock].detach();
}
void epioer::addsock(int isock)
{
    __sync_add_and_fetch(&m_socknum,1);

    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = isock;
    epoll_ctl(m_epfd,EPOLL_CTL_ADD,isock,&ev);
}
bool epollx::readx(int isock)
{
    return m_sockvec[isock].recv();
}
bool epollx::writex(int isock)
{
    return m_sockvec[isock].send();
}
void epollx::dispatch(int isock,unsigned int uiip,unsigned short usport)
{
    __sync_add_and_fetch(&m_socknum,1);
    if(m_sockvec[isock].bavail())
        m_sockvec[isock].detach();
    m_sockvec[isock].attach(isock,uiip,usport);

    int index = isock % (m_threadvec.size() - 1);
    epioer* piothread = (epioer*)m_threadvec[index];
    piothread->addsock(isock);
}

void eplister::mainfunction()
{
    g_log.log(LOG_LV_INFO,"eplister thread come!");
    int ifd = -1;
    sockaddr_in cliaddr;
    socklen_t addrlen = sizeof(cliaddr);

    //overtime handle thread;
	unsigned int uilastcheckalive = getticktime();
    while (isrun())
    {
        ifd = accept(m_listerfd,(sockaddr*)&cliaddr,&addrlen);
        if(ifd != -1)
        {
            g_log.log(LOG_LV_DEBUG,"link in,sock %d.",ifd);
            if (m_pep->bfull(ifd))
            {
                g_log.log(LOG_LV_DEBUG,"link refuse,sock %d.",ifd);
                close(ifd);
            }
            else
            {
               
                m_pep->dispatch(ifd,cliaddr.sin_addr.s_addr,ntohs(cliaddr.sin_port));
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                usleep(1 * 1000);
        }
		if (getinterval(uilastcheckalive,getticktime()) > (MAXDIETIME / 2))
		{
			m_pep->checkalive();
			uilastcheckalive = getticktime();
		}
    }
    g_log.log(LOG_LV_INFO,"eplister thread exit!");
}