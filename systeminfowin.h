#ifdef _WIN32
#ifndef _WIN_CPU_H_
#define _WIN_CPU_H_

#define _USE_32BIT_TIME_T 

#pragma comment(lib,"Iphlpapi.lib")

#include "windows.h"
#include "Iphlpapi.h"
#include <string>
#include <vector>
#include <new>
using namespace std;

typedef struct tagpare_net 
{
    DWORD dwsend;
    DWORD dwrecv;
}NET_PARE,*PNET_PARE;
// Use to convert bytes to KB
#define DIV 1024

LARGE_INTEGER liOldIdleTime = {0,0}; 
LARGE_INTEGER liOldSystemTime = {0,0};

#define SystemBasicInformation 0 
#define SystemPerformanceInformation 2 
#define SystemTimeInformation 3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct 
{ 
    DWORD dwUnknown1; 
    ULONG uKeMaximumIncrement; 
    ULONG uPageSize; 
    ULONG uMmNumberOfPhysicalPages; 
    ULONG uMmLowestPhysicalPage; 
    ULONG uMmHighestPhysicalPage; 
    ULONG uAllocationGranularity; 
    PVOID pLowestUserAddress; 
    PVOID pMmHighestUserAddress; 
    ULONG uKeActiveProcessors; 
    BYTE bKeNumberProcessors; 
    BYTE bUnknown2; 
    WORD wUnknown3; 
} SYSTEM_BASIC_INFORMATION;

typedef struct 
{ 
    LARGE_INTEGER liIdleTime; 
    DWORD dwSpare[76]; 
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct 
{ 
    LARGE_INTEGER liKeBootTime; 
    LARGE_INTEGER liKeSystemTime; 
    LARGE_INTEGER liExpTimeZoneBias; 
    ULONG uCurrentTimeZoneId; 
    DWORD dwReserved; 
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

PROCNTQSI NtQuerySystemInformation;

//获得CPU使用率
int GetCpuUsage()
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo; 
    SYSTEM_TIME_INFORMATION SysTimeInfo; 
    SYSTEM_BASIC_INFORMATION SysBaseInfo; 
    double dbIdleTime = 0; 
    double dbSystemTime = 0; 
    LONG status; 

    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll"),"NtQuerySystemInformation");

    if (!NtQuerySystemInformation) 
        return -1;

    // get number of processors in the system 
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL); 
    if (status != NO_ERROR) 
        return -1;

    // get new system time 
    status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0); 
    if (status!=NO_ERROR) 
        return -1;

    // get new CPU's idle time 
    status =NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL); 
    if (status != NO_ERROR) 
        return -1;

    // if it's a first call - skip it 
    if (liOldIdleTime.QuadPart != 0) 
    { 
        // CurrentValue = NewValue - OldValue 
        dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime); 
        dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

        // CurrentCpuIdle = IdleTime / SystemTime 
        dbIdleTime = dbIdleTime / dbSystemTime;

        // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors 
        dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

    }

    // store new CPU's idle and system time 
    liOldIdleTime = SysPerfInfo.liIdleTime; 
    liOldSystemTime = SysTimeInfo.liKeSystemTime;

    return (int)dbIdleTime;
}

int GetNumofProcessors()
{
    SYSTEM_INFO siSysInfo;

    GetSystemInfo(&siSysInfo); 
    return siSysInfo.dwNumberOfProcessors; 
}

// 返回可用物理内存,uiAllMemory为out型 所有物理内存。单位 KB
unsigned int GetMemoryStat(unsigned int & uiAllMemory)
{
    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof (statex);

    BOOL bret = GlobalMemoryStatusEx (&statex);
    if (bret == FALSE)
        return 0;
    uiAllMemory = (unsigned int)(statex.ullTotalPhys/DIV);
    return (unsigned int)(statex.ullAvailPhys/DIV);

    //     printf ("There is  %ld percent of memory in use.\n",
    //          statex.dwMemoryLoad);
    //     printf ("There are %I64d total Kbytes of physical memory.\n",
    //          statex.ullTotalPhys/DIV);
    //     printf ("There are %I64d free Kbytes of physical memory.\n",
    //          statex.ullAvailPhys/DIV);
    //     printf ("There are %I64d total Kbytes of paging file.\n",
    //          statex.ullTotalPageFile/DIV);
    //     printf ("There are %I64d free Kbytes of paging file.\n",
    //          statex.ullAvailPageFile/DIV);
    //     printf ("There are %I64d total Kbytes of virtual memory.\n",
    //          statex.ullTotalVirtual/DIV);
    //     printf ("There are %I64d free Kbytes of virtual memory.\n",
    //          statex.ullAvailVirtual/DIV);

    //     // Show the amount of extended memory available.
    // 
    //     printf ("There are %*I64d free Kbytes of extended memory.\n",
    //         WIDTH, statex.ullAvailExtendedVirtual/DIV);

}
// 返回整个空闲大小.uiAllDisk为硬盘总大小,和警告信息(如果某个分区剩余太少2%分界线) 单位MB
unsigned int GetDiskStat(unsigned int & uiAllDisk,string& strdescribe)
{
    strdescribe == "";
    uiAllDisk = 0;
    BOOL bret = FALSE;
    char chbuff[1024] = {0};
    DWORD dwlogic = GetLogicalDriveStrings(1023,chbuff);
    if (dwlogic == 0)
    {
        return 0;
    }
    else
    {
        ULARGE_INTEGER ulaall = {0},ulafree = {0};
        string strtmp = "";

        char* lpDriverBuffer = chbuff;
        while (*lpDriverBuffer != NULL)
        {
            char* pdiskstr  = lpDriverBuffer;

            ULARGE_INTEGER   FreeBytesAvailableToCaller,TotalNumberOfBytes,TotalNumberOfFreeBytes; 
            bret = GetDiskFreeSpaceEx(pdiskstr,&FreeBytesAvailableToCaller, 
                &TotalNumberOfBytes,&TotalNumberOfFreeBytes); 
            char outbuff[1024] = {0};
            if (bret)
            {
                ulafree.QuadPart += FreeBytesAvailableToCaller.QuadPart;

                ulaall.QuadPart += TotalNumberOfBytes.QuadPart;
                if ( (ulaall.QuadPart / ulafree.QuadPart ) >= 100 )
                {
                    strtmp += pdiskstr;
                    strtmp += " ";
                }
            }

            lpDriverBuffer = lpDriverBuffer + strlen(pdiskstr) + 1;
        }
        if (strtmp.length() >= 0)
        {
            strtmp += "空闲空间不足1%";
        }
        uiAllDisk = ulaall.QuadPart / (1024 * 1024);
        return ulafree.QuadPart / (1024 * 1024);
    }
}
bool  GetNetwork(vector<NET_PARE>& vec)  
{  
    vec.clear();
    DWORD                          dwIndex;  
    DWORD                          dwResult;  
    ULONG                          OutBufLen;  
    char*                pBuff = NULL;
    PIP_ADAPTER_INFO      pAdapterInfo = NULL;  
    PIP_ADAPTER_INFO    pAdapterNext;  
    MIB_IFROW                  IfRow;  

    OutBufLen  =  0;  
    dwResult    =  GetAdaptersInfo(pAdapterInfo,&OutBufLen);  
    if(dwResult  ==  ERROR_BUFFER_OVERFLOW)  
    {  
        pBuff = new(std::nothrow) char[OutBufLen];
        if (pBuff == NULL)
        {
            return false;
        }
        pAdapterInfo =(PIP_ADAPTER_INFO)pBuff;
        dwResult = GetAdaptersInfo(pAdapterInfo,&OutBufLen);  
        if (dwResult != ERROR_SUCCESS)
        {
            if (pBuff != NULL)
            {
                delete [] pBuff;
                pBuff = NULL;
            }

            return false;
        }
    }  
    else  if (dwResult  !=  ERROR_SUCCESS)  
    {  
        return  false;  
    }  
    pAdapterNext  =  pAdapterInfo;  

    while(pAdapterNext  !=  NULL)  
    {  
        dwIndex  =  pAdapterNext->Index;  

        IfRow.dwIndex  =  dwIndex;  
        if((dwResult    =  GetIfEntry(&IfRow))  !=  NO_ERROR)  
        {  
            break;  
        }  
        NET_PARE np;
        np.dwrecv = IfRow.dwInOctets;
        np.dwsend = IfRow.dwOutOctets;
        vec.push_back(np);
        pAdapterNext  =  pAdapterNext->Next;  
    }  
    if (pBuff != NULL)
    {
        delete [] pBuff;
        pBuff = NULL;
    }
    return  true;  
}  
unsigned int getinterval(unsigned int uilast,unsigned int uinow)
{
    if( uinow < uilast )
        return 0xFFFFFFFF - uilast + uinow;
    else
        return uinow - uilast;
}
// 单位KB
bool GetNetSpeed(unsigned int & uisendspeed,unsigned int & uirecvspeed)
{
    uisendspeed = 0;
    uirecvspeed = 0;

    vector<NET_PARE> vec1;

    bool bret = GetNetwork(vec1);

    if (bret == false)
    {
        return false;
    }
    DWORD dwbegtick = GetTickCount();
    Sleep(2000);
    DWORD dwendtick = GetTickCount();

    DWORD dwcost = getinterval(dwbegtick,dwendtick);

    vector<NET_PARE> vec2;
    bret = GetNetwork(vec2);
    if (bret == false)
    {
        return false;
    }

    if (vec2.size() != vec1.size())
    {
        return false;
    }
    DWORD dwsend = 0,dwrecv = 0;
    for (int i = 0; i != vec1.size(); ++i)
    {
        dwsend += (vec2[i].dwsend - vec1[i].dwsend);
        dwrecv += (vec2[i].dwrecv - vec1[i].dwrecv);
    }

    dwsend /= 1024; //KB
    dwrecv /= 1024; //KB

    dwsend *= 1000; //for divid millisecond
    dwrecv *= 1000;

    uisendspeed = dwsend / dwcost;
    uirecvspeed = dwrecv / dwcost;

    return true;
}
unsigned int GetTcpLink()
{
    MIB_TCPSTATS mts = {0};
    PMIB_TCPSTATS pTCPStats = &mts;
    DWORD dwRetVal = 0;
    if ((dwRetVal = GetTcpStatistics(pTCPStats)) == NO_ERROR) {
        return pTCPStats->dwNumConns;
    }
    return 0;
}
#endif
#endif