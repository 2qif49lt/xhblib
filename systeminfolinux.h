#ifndef _WIN32
#ifndef _LINUX_STATUS_H_
#define _LINUX_STATUS_H_

#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

typedef struct tagpare_net 
{
    long long uisend;
    long long uirecv;
}NET_PARE,*PNET_PARE;

typedef struct tag_cpuPACKED         //定义一个cpu occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned int user; //定义一个无符号的int类型的user
    unsigned int nice; //定义一个无符号的int类型的nice
    unsigned int system;//定义一个无符号的int类型的system
    unsigned int idle; //定义一个无符号的int类型的idle
}CPU_OCCUPY;

typedef struct tag_memPACKED         //定义一个mem occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned long total; 
    char name2[20];
    unsigned long free;                       
}MEM_OCCUPY;

void get_memoccupy (MEM_OCCUPY *mem) //对无类型get函数含有一个形参结构体类弄的指针O
{
    FILE *fd;          
    int n;             
    char buff[256];   
    MEM_OCCUPY *m;
    m=mem;

    fd = fopen ("/proc/meminfo", "r"); 

    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u %s", m->name, &m->total, m->name2); 
    fgets (buff, sizeof(buff), fd); //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里 
    sscanf (buff, "%s %u %s", m->name2, &m->free, m->name2); 

    fclose(fd);     //关闭文件fd
}

int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
    unsigned long od, nd;    
    unsigned long id, sd;
    int cpu_use = 0;   

    od = (unsigned long) (o->user + o->nice + o->system +o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (unsigned long) (n->user - o->user);    //用户第一次和第二次的时间之差再赋给id
    sd = (unsigned long) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
        cpu_use = (int)((sd+id)*100)/(nd-od);
    else cpu_use = 0;

    return cpu_use;
}

void get_cpuoccupy (CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{   
    FILE *fd;         
    char buff[256]; 
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;

    fd = fopen ("/proc/stat", "r"); 
    fgets (buff, sizeof(buff), fd);

    sscanf (buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);

    fclose(fd);     
}
int GetCpuUsage()
{
    CPU_OCCUPY c1 = {0},c2 = {0};
    get_cpuoccupy(&c1);
    sleep(2);
    get_cpuoccupy(&c2);

    return cal_cpuoccupy(&c1,&c2);
}
// 返回可用物理内存,uiAllMemory为out型 所有物理内存。单位 KB
unsigned int GetMemoryStat(unsigned int & uiAllMemory)
{
    MEM_OCCUPY mem_stat = {0};
    get_memoccupy(&mem_stat);
    uiAllMemory = mem_stat.total;
    return mem_stat.free;
}
bool GetDfInfo(string& strinfo)
{
    FILE* rs = NULL;  
    char* pbuf = new(nothrow) char[4096]; 
    if (pbuf == NULL)
    {
        return false;
    }
    memset(pbuf,0,4096);
    
    string strcmd = "df -m";
    string strtmpcmd = strcmd;
    rs = popen( strtmpcmd.c_str(), "r" ); 
    if (rs == NULL)
    {
        if (pbuf)
        {
            delete [] pbuf;
            pbuf = NULL;
        }
        return false;
    }

    fread(pbuf, sizeof(char), 4096 - 1,  rs);
    pclose( rs );  
    strinfo = pbuf;
    delete [] pbuf;
    pbuf = NULL;
    return true;
}
// 返回整个空闲大小.uiAllDisk为硬盘总大小,和警告信息(如果某个分区剩余太少2%分界线) 单位MB
unsigned int GetDiskStat(unsigned int & uiAllDisk,string& strdescribe)
{
    //  文件系统	         1M-块      已用      可用 已用% 挂载点
    //  /dev/sda3               344508      5054    321954   2% /
    //  tmpfs                     1643         1      1643   1% /dev/shm
    //  /dev/sda1                54137       204     51184   1% /boot
    strdescribe = "";
    uiAllDisk = 0;
    string strdf;
    bool bret = GetDfInfo(strdf);
    if (bret == false)
    {
        return 0;
    }
    stringstream ss(strdf);
    
    string strline;
    getline(ss,strline,'\n');
    
    unsigned int uifree = 0;
    while (getline(ss,strline,'\n'))
    {
        stringstream ssline(strline);
        string strfsname;
        unsigned int uifsall = 0, uifsfee = 0;
        ssline >> strfsname >> uifsall >> uifsfee >> uifsfee;
        if (uifsall == 0 || (uifsfee * 100 / uifsall <= 2))
        {
            strdescribe += strfsname;
            strdescribe += ' ';
        }
        uiAllDisk += uifsall;
        uifree += uifsfee;
    }
    if (strdescribe.length() > 0)
    {
        strdescribe += "空闲空间不足1%";
    }
    return uifree;
}
bool GetNetDevInfo(string& strinfo)
{
    FILE* rs = NULL;  
    char* pbuf = new(nothrow) char[4096]; 
    if (pbuf == NULL)
    {
        return false;
    }
    memset(pbuf,0,4096);

    string strcmd = "cat /proc/net/dev";
    string strtmpcmd = strcmd ;
    rs = popen( strtmpcmd.c_str(), "r" ); 
    if (rs == NULL)
    {
        if (pbuf)
        {
            delete [] pbuf;
            pbuf = NULL;
        }
        return false;
    }

    fread(pbuf, sizeof(char), 4096 - 1,  rs);
    pclose( rs );  
    strinfo = pbuf;
    delete [] pbuf;
    pbuf = NULL;
    return true;
}
string& stltrim(string& strtxt)
{
    strtxt.erase(
        strtxt.begin(),
        find_if(strtxt.begin(),strtxt.end(),not1(ptr_fun(::isspace))));
    strtxt.erase(
        find_if(strtxt.rbegin(),strtxt.rend(),not1(ptr_fun(::isspace))).base(),
        strtxt.end());
    return strtxt;
}
bool  GetNetwork(vector<NET_PARE>& vec)
{
     /*
    [root@xuhaibin ~]# cat /proc/net/dev
    Inter-|   Receive                                                |  Transmit
    face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo: 1312928   36344    0    0    0     0          0         0  1312928   36344    0    0    0     0       0          0
    eth0:388266686 3567787    0    0    0     0          0         0 17348929  128179    0    0    0     0       0          0
    */
    vec.clear();
    string strcat;
    bool bret = GetNetDevInfo(strcat);
    if (bret == false)
    {
        return false;
    }

    stringstream ss(strcat);
    string strline;
    long long uirecv = 0, uisend = 0;
    long long uiignore;
    getline(ss,strline,'\n');
    getline(ss,strline,'\n');
    while (getline(ss,strline,'\n'))
    {
        stringstream ssline(strline);
        string strfacename;
        getline(ssline,strfacename,':');
        stltrim(strfacename);
        if (strfacename == "lo")    // ignore local loopback
        {
            continue;
        }
        ssline>>uirecv>>uiignore>>uiignore>>uiignore>>uiignore>>uiignore>>
            uiignore>>uiignore>>uisend;
        NET_PARE np;
        np.uisend = uisend;
        np.uirecv = uirecv;
        vec.push_back(np);
    }
    return true;
}
unsigned int gettick()
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
bool GetNetSpeed(unsigned int & uisendspeed,unsigned int & uirecvspeed)
{
    uisendspeed = 0;
    uirecvspeed = 0;
    
    vector<NET_PARE> vec1,vec2;
    bool bret = GetNetwork(vec1);
    if (bret == false)
    {
        return false;
    }
    unsigned int uilasttick = gettick();
    sleep(2);
    unsigned int uinowtick = gettick();
    bret = GetNetwork(vec2);
    if (bret == false)
    {
        return false;
    }
    unsigned int uicosrmilli = getinterval(uilasttick,uinowtick);

    if (vec2.size() != vec1.size())
    {
        return false;
    }
    long long uisend = 0,uirecv = 0;
    for (int i = 0; i != vec1.size(); ++i)
    {
        uisend += (vec2[i].uisend - vec1[i].uisend);
        uirecv += (vec2[i].uirecv - vec1[i].uirecv);
    }

    uisend /= 1024; //KB
    uirecv /= 1024; //KB

    uisend *= 1000; //for divid millisecond
    uirecv *= 1000;

    uisendspeed = uisend / uicosrmilli;
    uirecvspeed = uirecv / uicosrmilli;
    return true;
}
bool GetNetstatLink(string& strnum)
{
    FILE* rs = NULL;  
    char* pbuf = new(nothrow) char[4096]; 
    if (pbuf == NULL)
    {
        return false;
    }
    memset(pbuf,0,4096);

    string strcmd = "netstat -n | awk '/^tcp/ {++state[$NF]} END {for(key in state) print state[key]}'";
    string strtmpcmd = strcmd ;
    rs = popen( strtmpcmd.c_str(), "r" ); 
    if (rs == NULL)
    {
        if (pbuf)
        {
            delete [] pbuf;
            pbuf = NULL;
        }
        return false;
    }

    fread(pbuf, sizeof(char), 4096 - 1,  rs);
    pclose( rs );  
    strnum = pbuf;
    delete [] pbuf;
    pbuf = NULL;
    return true;
}
unsigned int GetTcpLink()
{
    string strnum;
    bool bret = GetNetstatLink(strnum);
    if (bret == false)
    {
        return 0;
    }
    stringstream ss(strnum);
    string strline;
    unsigned uiall = 0;
    while (getline(ss,strline,'\n'))
    {
        uiall += atoi(strline.c_str());
    }
    return uiall;
}
#endif
#endif