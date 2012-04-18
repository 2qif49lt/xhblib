#include "epollx.h"

logx<mlockx_> g_log;

char* alloc(int n)
{
    if(n > 0)
        return new(nothrow) char[n];
}
void release(char* pmem)
{
    if(pmem != NULL)
        delete [] pmem;
}

bool setnoblock(int isock)
{
    int opts = fcntl(isock,F_GETFL);
    if (opts < 0)
        return false;

    opts |= O_NONBLOCK;

    return fcntl(isock,F_SETFL,opts) != -1;
}

bool sockx::recv()
{
    const int ibufflen = 1024;

    bool bread = true;
    while(bread)
    {
  //      g_log.log(LOG_LV_DEBUG,"sockid %d recv.",m_isock);

        pmsgx pm = (pmsgx)alloc(sizeof(msgx));
        if(pm == NULL)
            return false;
        char * buff = alloc(ibufflen);
        if(buff == NULL)
        {
            release((char*)pm);
            return false;
        }
        pm->pdata = buff;
        pm->ilen = 0;
        pm->isend = 0;

        int ipr = 0,inr = ibufflen,ihr = 0;
        while (ihr < inr)
        {
            ipr = read(m_isock,pm->pdata + ihr,inr);
            if (ipr == 0)
            { 
                g_log.log(LOG_LV_DEBUG,"read ret 0,errno:%d.",errno);
                release((char*)pm); 
                release(buff);
                return false;
            }
            if (ipr == -1)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    bread = false;
                    break;			
                }
                else
                {
                    g_log.log(LOG_LV_DEBUG,"read ret -1 and errno != EAGAIN,errno:%d.",errno);
                    release((char*)pm);
                    release(buff);
                    return false;
                }
            }
            else
            {
                ihr += ipr;
                if(ipr < inr)
                {
                    bread = false;
                    break;
                }
                inr -= ipr;
            }
        }

        pm->ilen = ihr;
        if(m_rsq.isfull())
            ++m_iab;
        else
            m_rsq.push_back(pm);
    }
    return true;
}

bool sockx::send()
{
 //   g_log.log(LOG_LV_DEBUG,"sockid %d send.",m_isock);
    pmsgx pm = NULL; 
    while(pm = m_rsq.front())
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
        m_rsq.pop_front();
        release((char*)pm);
        release(buff);
    }
    return true;
}

void sockx::detach()
{
    if(m_isock > 0)
        close(m_isock);
    m_isock = -1;
    pmsgx pm = NULL;
    while(pm = m_rsq.pop_front())
    {
        release(pm->pdata);
        release((char*)pm);
    }
}

sockx::~sockx()
{
    detach();
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
void epollx::dispatch(int isock)
{
    __sync_add_and_fetch(&m_socknum,1);
    if(m_sockvec[isock].bavail())
        m_sockvec[isock].detach();
    m_sockvec[isock].attach(isock);

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
    while (isrun())
    {
        ifd = accept(m_listerfd,(sockaddr*)&cliaddr,&addrlen);
        if(ifd != -1)
        {
            g_log.log(LOG_LV_DEBUG,"link in,sock %d.",ifd);
            if (m_pep->bfull())
            {
                g_log.log(LOG_LV_DEBUG,"link refuse,sock %d.",ifd);
                close(ifd);
            }
            else
                m_pep->dispatch(ifd);
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                usleep(1 * 1000);
        }
    }
    g_log.log(LOG_LV_INFO,"eplister thread exit!");
}