#ifndef WMIX_SYSTEM_XHB_H_
#define WMIX_SYSTEM_XHB_H_



#define _WIN32_DCOM 
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include "commonx.h"
#include "lockx.h" //线程锁

#include <string>
#include <vector>
#include <map>
using std::string;
using std::wstring;
using std::vector;
using std::map;

namespace xhb
{
	struct WMI_SIMPLE_PROCESS_INFO 
	{
		wstring Caption;
		wstring CreationDate;	//时间为201007251138 年月日时分固定程度 12字节
		wstring ExecutablePath;

		unsigned int ProcessID;

		unsigned int ThreadCount;

	};
	struct WMI_ACCOUNT_INFO
	{
		wstring Name;
		int	Disabled;	//是否禁用
		int PasswordChangeable;
		int PasswordExpires;
		int PasswordRequired;
	};
	struct Per_Cnt_Enumerator
	{
		void					*pEnum;
		long					lID;
	};

	union unint
	{
		DWORD  t;
		UINT64 s;
	};

	struct Multi_Property
	{
		wstring		name;
		int			type;
		unint		value;
	};

	class IWmiOperator
	{
		
	public:
		IWmiOperator();

		virtual ~IWmiOperator();


		static BOOL CoInitialize();			//程序开始时应该调用这个函数。

		static void CoUninitialize();		//程序结束前应该调用这个函数

	public:
		
		DWORD GetLastWMIError(){return m_LastErrorNum;}

		string GetLastWMIDesc(){return m_LastErrorStr;}

		BOOL InitWmiInterface();			// 初始化COM和设置安全等级。


		BOOL ConnectServer(wstring ip = L"",wstring name = L"",wstring pw =L"");		//获取IP 帐号密码 ，并且进行连接。如果是连接本机的话，ip参数投递空的字符串。


		//////////////////////////////////////////////////////////////////////////
		//系统

		BOOL GetSystemInfo(wstring & strout);						//获取操作系统的详细数据


		//////////////////////////////////////////////////////////////////////////
		//CPU

		BOOL GetCpuInfo(vector<wstring> & vec);						//获取各个CPU的详细数据。

		BOOL GetCpuDeviceIDVec(vector<wstring> &vec);					//获取CPU的主键,比如:"CPU0","CPU1"

		BOOL GetSingleCpuLoadPercentage(const wstring &DeviceID,unsigned short & out);							//获取某个cpu的使用率。该函数不能频繁调用。

		BOOL GetCpuUsage(unsigned long & dwHZCpu);		//获取CPU频率

		BOOL GetCpuPercent(int &iPercent);			//现在用这个获取CPU使用率

		BOOL GetCpuID(wstring & strID);			//获取CPU ID 全球唯一

		BOOL AddCpuPerCnt()
		{

			wstring classname = L"Win32_PerfFormattedData_PerfOS_Processor";

			return AddPerCntEnumeratorByClass(classname);

		}

		BOOL DelCpuPerCnt()
		{
			wstring classname = L"Win32_PerfFormattedData_PerfOS_Processor";

			return DelPerCntEnumeratorByClass(classname);
		}

		IWbemHiPerfEnum * GetCpuPerCntEnum()
		{
			AddCpuPerCnt();

			wstring classname = L"Win32_PerfFormattedData_PerfOS_Processor";

			return (IWbemHiPerfEnum *)m_EnumMap[classname].pEnum;
		}



		BOOL GetCpuPerCnt(IWbemHiPerfEnum * pEnum,const wstring & member,vector<unint> &out,int type);

			
		//////////////////////////////////////////////////////////////////////////
		//内存
		
		BOOL GetPhysicalMemory(vector<wstring> &vec);				//获取物理内存设备的数据：。

		BOOL GetPhysicalMemoryUsage(unsigned long & dwAllMemory,int & nUseMem_Percent);		//获取内存的大小和空闲率

		BOOL GetPageFile(vector<wstring> &vec);						//获取分页文件信息：虚拟内存


		BOOL AddMemoryPerCnt();										//初始化内存性能计数：如果要使用内存性能计数，则需要先调用这个函数。

		BOOL DelMemoryPerCnt();

		IWbemHiPerfEnum * GetMemoryPerCntEnum()
		{
			AddMemoryPerCnt();

			wstring classname = L"Win32_PerfRawData_PerfOS_Memory";

			return (IWbemHiPerfEnum *)m_EnumMap[classname].pEnum;
		}

		BOOL GetMemoryPerCnt(IWbemHiPerfEnum * pEnum,const wstring & member,void *out,int type);	//获取member对应的成员的性能计数。 type表示成员的数据类型。


		//////////////////////////////////////////////////////////////////////////
		//磁盘
		
		BOOL GetDiskVector(vector<wstring> &vec);
		
		BOOL GetDiskUsage(unsigned long & dwAllSpace,int & nUsage,BOOL & bHaveDiskFull,string & strSingleDisk);			//获取磁盘总的大小 单位M，和使用百分比,第3个参数是返回快满的分区.如:CDEFG

		//////////////////////////////////////////////////////////////////////////
		//帐户控制

		BOOL CheckAccountExist(const wstring &name,BOOL &bExist);	//检查某个帐户是否存在，bExist表示是否存在。

		BOOL GetAccountVec(vector<WMI_ACCOUNT_INFO> &AccountVec);

		BOOL GetAccountVector( vector<wstring> &AccountVec);		//每个帐户详细信息为一个格式化好的字符串成员。



		BOOL AddAccount(const wstring &namepre ,wstring &outname, wstring &outpw);					//添加一个账户，namepre为外部提供的名字前缀（比如:xhb），后面2个参数为注册好的账户名和密码。

		BOOL DelAccount(const wstring &name);														//删除一个账户。



		BOOL SetAccountGroup(const wstring &accountname,const wstring & groupname = L"administrators");		//添加某账户加入某组，默认是超级管理员组。

		BOOL DelAccountGroup(const wstring &accountname,const wstring & groupname = L"administrators");		//同上的反义词。


		BOOL SetAccountPassword(const wstring &accountname,wstring & password);						//修改指定账户的密码，可能会返回成功但实际上是失败的。
																									//如果password为空字符串，则函数成功后在这个参数返回新密码。


																									
		//////////////////////////////////////////////////////////////////////////
		//服务控制
		
		BOOL GetServiceVector( vector<wstring> &ServiceVec);		//获取系统的服务详细信息。

		BOOL GetSingleServiceInfo(const wstring &ProcessId,wstring & Info);			//获取ProcessId标识的服务信息,如果ProcessID的服务不存在，Info则会返回一个提醒。



		BOOL StartService1(const wstring &srvname);					//启动一项服务。

		BOOL StopService(const wstring &srvname);					//停止一项服务。
			
		BOOL PauseService(const wstring &srvname);					//暂停

		BOOL ResumeService(const wstring &srvname);					//继续



		BOOL DeleteService(const wstring &srvname);					//删除


		BOOL SetServiceStartMode(const wstring &srvname, const wstring &mode);		//设置启动方式。



		//////////////////////////////////////////////////////////////////////////
		//进程控制
		/*
		如：
		Caption = "winlogon.exe";
		CreationDate = "20100725113830.984375+480";
		ExecutablePath = "C:\\WINDOWS\\system32\\winlogon.exe";
		ProcessId = 320;
		ThreadCount = 17;
		*/


		BOOL GetProcessSimpleInfoVector( vector<WMI_SIMPLE_PROCESS_INFO> &ProcessVec);	//获取简单的进程信息列表。


		BOOL GetProcessVector( vector<wstring> &ProcessVec);		//获取系统的进程详细信息表。

		
		BOOL GetSingleProcessInfo(const wstring &ProcessId,wstring & Info);			//获取ProcessId标识的进程信息,如果ProcessID的进程不存在，Info则会返回一个提醒。

		
		BOOL CreateProcess(const wstring & processcmdline);			//这个参数processcmdline是程序的路径 加参数，比如: processcmdline = L"C:\\WINDOWS\\notepad.exe  -o -x -ooxx";
																	//这个函数应该也可以用来注册服务。比如:processcmdline = L""F:\\Gene6 FTP Server\、G6FTPServer.exe" -install";
		
		BOOL TerminateProcess(const wstring & processid);			//强制结束该进程，processid 为 pid;

		
		//////////////////////////////////////////////////////////////////////////
		//网络
		/************************************************************************/
		/* 
		Win32_PerfRawData_Tcpip_IP
		Win32_PerfRawData_Tcpip_NBTConnection
		Win32_PerfRawData_Tcpip_TCP
		Win32_PerfRawData_Tcpip_UDP    

		注意：win2003的类名不同Win32_PerfRawData_Tcpip_IPV4

		Win32_PerfRawData_Tcpip_NetworkInterface 为网卡流量
		*/
		/************************************************************************/
		BOOL AddIpPerCnt()
		{
			
			wstring classname = L"Win32_PerfRawData_Tcpip_IP";

			if (m_WindowMajorVersion == 5 && m_WindowMinorVersion == 2)
			{
				classname += L"V4";
			}

			return AddPerCntEnumeratorByClass(classname);
		}

		BOOL DelIpPerCnt()
		{
			wstring classname = L"Win32_PerfRawData_Tcpip_IP";

			if (m_WindowMajorVersion == 5 && m_WindowMinorVersion == 2)
			{
				classname += L"V4";
			}

			return DelPerCntEnumeratorByClass(classname);
		}

		IWbemHiPerfEnum *GetIPPerCntEnum()
		{
			AddIpPerCnt();

			wstring classname = L"Win32_PerfRawData_Tcpip_IP";

			if (m_WindowMajorVersion == 5 && m_WindowMinorVersion == 2)
			{
				classname += L"V4";
			}

			return (IWbemHiPerfEnum *)m_EnumMap[classname].pEnum;
		}

		BOOL AddFlowRatePerCnt()	//添加流量性能计数
		{

			wstring classname = L"Win32_PerfRawData_Tcpip_NetworkInterface";

			return AddPerCntEnumeratorByClass(classname);
		}

		BOOL DelFlowRatePerCnt()			
		{
			wstring classname = L"Win32_PerfRawData_Tcpip_NetworkInterface";

			return DelPerCntEnumeratorByClass(classname);
		}

		IWbemHiPerfEnum *GetFlowRatePerCntEnum()
		{
			AddIpPerCnt();

			wstring classname = L"Win32_PerfRawData_Tcpip_NetworkInterface";

			return (IWbemHiPerfEnum *)m_EnumMap[classname].pEnum;
		}
	/*
		IWbemObjectAccess *GetIPPerCntAccess()
		{
			AddIpPerCnt();
			wstring pathname = L"Win32_PerfRawData_Tcpip_IP";

			return (IWbemObjectAccess *)m_EnumMap[pathname].pEnum;
		}

		
	*/
		BOOL GetMultiPerCntVec(IWbemHiPerfEnum *pAcc,vector<Multi_Property> &inandoutvec);
		
		BOOL GetMultiPerCntVec(IWbemObjectAccess *pAcc,vector<Multi_Property> &inandoutvec);

		BOOL GetTCPConnectNum(unsigned int & num);

		BOOL GetNetSpeed(unsigned int &BytesReceivedPerSec,unsigned int &BytesSentPerSec );	// 获取每秒的进出字节数
	private:

		BOOL InitPerCnt();								//初始化性能计数器接口。

		BOOL AddPerCntEnumeratorByClass(const wstring &classname) ;					//添加计数器。一般的类用这个

		BOOL DelPerCntEnumeratorByClass(const wstring &classname);					//去除计数器。


		BOOL AddPerCntEnumeratorByPath(const wstring & pathname);					//主要用于实时获取某个进程或服务的数据。

		BOOL DelPerCntEnumeratorByPath(const wstring & pathname);


		BOOL SetSecurityLevel(IUnknown *pProxy);		//设置安全等级，传入的参数可能是不同种类的。

		BOOL ExecQuery(const wstring &wqlcmd);			//执行命令后，结果集在m_pEnumerator内。

		wstring StringCreatePolicy(const wstring &type = L"Password");			// 用来生成账户的名字，密码等。 type=L"Administrator" 为生成用户名

		BOOL ExecCreateProcess(const wstring &cmd);		//目前专门用来执行一些创建进程的命令。比如：创建一个net进程，命令行为 net user ooxxoox oxoxox /add 。

		BOOL ExecSelect(const wstring &strcmd,vector<wstring> &vec);			//目前专门用来执行一些select命令，然后将结果放在vec里。


		BOOL ExecService(const wstring &name ,const wstring &oper ,const wstring &operpara = L"");			//目前用来执行服务方面的操作。name为服务的名字（主键，不是display name），
																											//oper表示操作（win32_service的方法） operpara表示参数
																											//这里只有一个方法changestartmode需要一个参数。

		

	private:
		
		IWbemLocator			*m_pLoc;		// wmi locator

		IWbemServices			*m_pSvc;		// wmi namespace pointer

		IEnumWbemClassObject	*m_pEnumerator;	// something like recordset

		IWbemRefresher          *m_pRefresher;

		IWbemConfigureRefresher *m_pConfig;

	private:

		string		m_LastErrorStr;

		DWORD		m_LastErrorNum;

	private:

		wstring		m_StrIP;			//目标服务器的IP

		wstring		m_StrAdmin;			//administrators 级别的帐户
		
		wstring		m_StrPW;			//上面对应的密码

	private:

		IWmiOperator(const IWmiOperator &){}

		IWmiOperator& operator= (const IWmiOperator &){}
		

	private:

		map<wstring,Per_Cnt_Enumerator>	m_EnumMap;

		criticalsectionx_ m_lockobject;

		DWORD m_WindowMajorVersion;		//XP的主版本号5 子版本号1 ，2003 为5.2

		DWORD m_WindowMinorVersion;

	};

};
#endif			//WMIX_SYSTEM_XHB_H_