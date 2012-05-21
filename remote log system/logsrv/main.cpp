#include "epollx.h"
int main()
{
    g_log.InitLog("log","logsrv.log",LOG_LV_DEBUG);
    g_log.log("logsv begin",LOG_LV_DEBUG);

    epollx ep;
    bool bret = ep.start(NULL,9898,1000);
    while (1)
    {
        sleep(1);
    }
    return 0;
}