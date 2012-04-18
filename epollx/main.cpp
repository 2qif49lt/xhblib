#include "epollx.h"
int main()
{
    g_log.InitLog("log","epoll.log",LOG_LV_DEBUG);
    g_log.log("hello",LOG_LV_DEBUG);

    epollx ep;
    bool bret = ep.start(NULL,9898,1000);
    while (1)
    {
        sleep(1);
    }
    return 0;
}