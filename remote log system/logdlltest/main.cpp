#pragma comment(lib,"logdll.lib")
#include "../logdll/logdll/remotelog_if.h"
#include <stdio.h>
#include <windows.h>
int main()
{
    int iret = init_rlog(1,"10.0.90.159","10.0.90.56",9898,5);
    if (iret != 0)
    {
        printf("init_rlog error.\n");
        goto err;
    }
    char buff[100];
    int i = 0;
    Sleep(3000);
    while (++i < 10000)
    {
        sprintf(buff,"%08d hello world!",i);
        iret = send_rlog(3,buff);
        if (iret != 0)
        {
            printf("send_rlog error.\n");
            goto err;
        }
        Sleep(1);
    }
    stop_rlog();
err:
    {
        printf("err iret %d\n",iret);
        exit(0);
    }
    return 0;
}