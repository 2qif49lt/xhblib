#ifdef _WIN32
#include "winsysteminfo.h"
#else
#include "linuxsysteminfo.h"
#endif
#include <stdio.h>
#include <sstream>
#include <iostream>
using namespace std;


/*
int main()
{
/*    while (1)*/
    {
        int icpu = 0;
        icpu = GetCpuUsage();
    

        unsigned int uiavailmem = 0,uiallmem = 0;
        uiavailmem = GetMemoryStat(uiallmem);
        

        string str;
        unsigned int uifd = 0;  
        unsigned int uiad = 0;
        uifd = GetDiskStat(uiad,str);


        unsigned int uis,uir;
        GetNetSpeed(uis,uir);

        unsigned int uilink = GetTcpLink();

        stringstream ss;
        ss<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss<<"<stat>\n";
        ss<<"\t<cpu>"<<icpu<<"</cpu>\n";
        ss<<"\t<freememory>"<<uiavailmem<<"</freememory>\n";
        ss<<"\t<allmemory>"<<uiallmem<<"</allmemory>\n";
        ss<<"\t<freedisk>"<<uifd<<"</freedisk>\n";
        ss<<"\t<alldisk>"<<uiad<<"</alldisk>\n";
        ss<<"\t<warnmsg>"<<str<<"</warnmsg>\n";
        ss<<"\t<sendspeed>"<<uis<<"</sendspeed>\n";
        ss<<"\t<recvspeed>"<<uir<<"</recvspeed>\n";
        ss<<"\t<tcplink>"<<uilink<<"</tcplink>\n";
        ss<<"</stat>";
        cout<<ss.str()<<endl;
    }
    
    return 0;
}
*/