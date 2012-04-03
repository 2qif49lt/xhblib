#include "wmix.h"
#pragma warning(push)
#pragma warning(disable : 4244)	//“unsigned __int64”转换到“int”，可能丢失数据
#pragma warning(disable : 4996)	//去掉安全函数警告
#pragma warning(disable : 4018)	//去掉signed 和 unsigned转换警告
namespace xhb
{

	IWmiOperator::IWmiOperator()
	{
		m_LastErrorNum = 0;
		m_pLoc = NULL;

		m_pSvc = NULL;

		m_pEnumerator = NULL;

		m_pRefresher = NULL;

		m_pConfig = NULL;

		DWORD dwVersion;

		dwVersion = GetVersion();

		// Get the Windows version.

		m_WindowMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

		m_WindowMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		
	}

	IWmiOperator::~IWmiOperator()
	{
		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		if (m_pSvc != NULL)
		{

			m_pSvc->Release();

			m_pSvc = NULL;

		}

		if (m_pLoc != NULL)
		{

			m_pLoc->Release();

			m_pLoc = NULL;

		}

		if (m_pRefresher != NULL)
		{
			m_pRefresher->Release();

			m_pRefresher = NULL;
		}

		if (m_pConfig != NULL)
		{
			m_pConfig->Release();

			m_pConfig = NULL;
		}

	/*
			for (map<wstring,Per_Cnt_Enumerator>::iterator it = m_EnumMap.begin();it != m_EnumMap.end();++it)
			{
		
				Per_Cnt_Enumerator &pcen = it->second;
		
				if (pcen.pEnum != NULL)
				{
					pcen.pEnum->Release();
					
					pcen.pEnum = NULL;
				}
			}*/
		
	}
	BOOL IWmiOperator::CoInitialize()
	{

		// Step 1: --------------------------------------------------

		// Initialize COM. ------------------------------------------


		HRESULT hres;

		hres =  CoInitializeEx(0, COINIT_MULTITHREADED);

		if (FAILED(hres))

		{
			return FALSE;                  // Program has failed.

		}

		// Step 2: --------------------------------------------------

		// Set general COM security levels --------------------------

		// Note: If you are using Windows 2000, you need to specify -

		// the default authentication credentials for a user by using

		// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----

		// parameter of CoInitializeSecurity ------------------------



		hres =  CoInitializeSecurity(

			NULL,

			-1,                          // COM authentication

			NULL,                        // Authentication services

			NULL,                        // Reserved

			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication

			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation 

			NULL,                        // Authentication info

			EOAC_NONE,                   // Additional capabilities

			NULL                         // Reserved

			);


		if (FAILED(hres))

		{
			return FALSE;                    // Program has failed.

		}

		return TRUE;
	}

	void IWmiOperator::CoUninitialize()
	{
		::CoUninitialize();
	}
	BOOL IWmiOperator::InitWmiInterface()
	{
		HRESULT hres;



		hres = CoCreateInstance(

			CLSID_WbemLocator,            

			0,

			CLSCTX_INPROC_SERVER,

			IID_IWbemLocator, (LPVOID *) &m_pLoc);



		if (FAILED(hres))

		{

			m_LastErrorStr = "Failed to create IWbemLocator object.";

			m_LastErrorNum = hres;

			return FALSE;                 // Program has failed.

		}
		

		return InitPerCnt();
	}




	BOOL IWmiOperator::ConnectServer(wstring ip,wstring name,wstring pw)
	{

		m_StrIP = ip;

		m_StrAdmin = name;

		m_StrPW = pw;


		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		if (m_pSvc != NULL)
		{

			m_pSvc->Release();

			m_pSvc = NULL;

		}



		HRESULT hres;

		if (m_StrIP == L"")		//连接本地
		{

			hres = m_pLoc->ConnectServer(

				L"root\\cimv2",

				NULL,							// User name

				NULL,								// User password

				0,									// Locale            

				NULL,								// Security flags

				NULL,								// Authority       

				0,									// Context object

				&m_pSvc                             // IWbemServices proxy

				);

		}
		else
		{

			wstring strtmp = L"\\\\";

			strtmp += m_StrIP;

			strtmp += L"\\root\\cimv2";

			hres = m_pLoc->ConnectServer(

				const_cast<WCHAR *>(strtmp.c_str()),

				const_cast<WCHAR *>(m_StrAdmin.c_str()),							// User name

				const_cast<WCHAR *>(m_StrPW.c_str()),								// User password

				0,									// Locale            

				NULL,								// Security flags

				NULL,								// Authority       

				0,									// Context object

				&m_pSvc                             // IWbemServices proxy

				);

		}
		


		if (FAILED(hres))

		{

			m_LastErrorStr = "Could not connect.";

			m_LastErrorNum = hres;
			
			return FALSE;                // Program has failed.

		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::SetSecurityLevel(IUnknown *pProxy)
	{

		WCHAR pszDom[100]  = L"DomainOrRemote"; 

		COAUTHIDENTITY cID;


		cID.User           = (USHORT*)(m_StrAdmin.c_str());

		cID.UserLength     = m_StrAdmin.length();	//bstrUsername.length();

		cID.Password       = (USHORT*)(m_StrPW.c_str());

		cID.PasswordLength = m_StrPW.length();		//bstrPassword.length();

		cID.Domain         = (USHORT*)&pszDom;

		cID.DomainLength   = lstrlenW(pszDom);		// bstrDomain.length();

		cID.Flags          = SEC_WINNT_AUTH_IDENTITY_UNICODE;

		
		COAUTHIDENTITY *pCid = &cID;

		if (m_StrIP.length() == 0)
		{

			pCid = NULL;
		}


		HRESULT hres = CoSetProxyBlanket(

			pProxy,							// Indicates the proxy to set

			RPC_C_AUTHN_WINNT,				 // RPC_C_AUTHN_xxx

			RPC_C_AUTHZ_NONE,				// RPC_C_AUTHZ_xxx

			NULL,							 // Server principal name

			RPC_C_AUTHN_LEVEL_CALL,			 // RPC_C_AUTHN_LEVEL_xxx

			RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx

			pCid,							// client identity

			EOAC_NONE						// proxy capabilities

			);



		if (FAILED(hres))

		{
			   
			m_LastErrorStr = "Could not set proxy blanket.";

			m_LastErrorNum = hres;

			return FALSE;

		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::ExecQuery(const wstring &wqlcmd)
	{
		BOOL ret = FALSE;

		ret = SetSecurityLevel(m_pSvc);
		
		if (ret == FALSE)
		{
			return FALSE ;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		HRESULT hres = m_pSvc->ExecQuery(

			bstr_t(L"WQL"),

			bstr_t(wqlcmd.c_str()),

			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,

			NULL,

			&m_pEnumerator);



		if (FAILED(hres))

		{
			m_LastErrorStr = "ExecQuery failed.";

			m_LastErrorNum = hres;

			return FALSE;               // Program has failed.

		}
		
		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::CheckAccountExist(const wstring &name,BOOL &bExist)
	{
		wstring strcmd = L"Select * from Win32_UserAccount where Name=\"";

		strcmd += name;

		strcmd += L"\"";

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}
		
		IWbemClassObject *pclsObj = NULL;

		DWORD uReturn = 0;

		HRESULT hres = m_pEnumerator->Next(WBEM_INFINITE, 1,

			&pclsObj, &uReturn);


		if (FAILED(hres))
		{
			if (pclsObj != NULL)
			{

				pclsObj->Release();

				pclsObj = NULL;

			}

			if (m_pEnumerator != NULL)
			{

				m_pEnumerator->Release();

				m_pEnumerator = NULL;

			}

			m_LastErrorStr = "an error appear in the final step when read the m_pEnumerator (recordset)!";

			m_LastErrorNum = hres;

			return FALSE;
		}


		if(0 == uReturn)

		{
			bExist = FALSE;
			
		}
		else
		{
			bExist = TRUE;
		}


		if (pclsObj != NULL)
		{

			pclsObj->Release();

			pclsObj = NULL;

		}

		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::GetAccountVec(vector<WMI_ACCOUNT_INFO> &AccountVec)
	{
		wstring strcmd = L"select * from Win32_UserAccount";

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);

			if(0 == uReturn)

			{
				break;
			}

			WMI_ACCOUNT_INFO info;

			VARIANT  Var;

			VariantClear(&Var);

			hr = pclsObj->Get(L"Name",0,&Var,NULL,NULL);

			info.Name = Var.bstrVal;

			VariantClear(&Var);

			hr = pclsObj->Get(L"Disabled",0,&Var,NULL,NULL);

			info.Disabled = (Var.boolVal != 0 ? 1 : 0);


			VariantClear(&Var);

			pclsObj->Get(L"PasswordChangeable",0,&Var,NULL,NULL);

			info.PasswordChangeable = (Var.boolVal != 0 ? 1 : 0);
		
			VariantClear(&Var);

			pclsObj->Get(L"PasswordExpires",0,&Var,NULL,NULL);

			info.PasswordExpires = (Var.boolVal != 0 ? 1 : 0);

			VariantClear(&Var);

			pclsObj->Get(L"PasswordRequired",0,&Var,NULL,NULL);

			info.PasswordRequired = (Var.boolVal != 0 ? 1 : 0);

			AccountVec.push_back(info);


			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}
		}

		

		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}
	BOOL IWmiOperator::GetAccountVector(vector<wstring> &AccountVec)
	{

		//////////////////////////////////////////////////////////////////////////
		/*
		
		//目前只返回这个信息，需要单独的属性需要对这个字符串进行查找。比如要得到用户名，可以这样： str.find(L"Name =");得到索引 然后做下一步操作。
		//vector 内的成员字符串具体为如下格式

		instance of Win32_UserAccount
		{
			AccountType = 512;
			Caption = "XUHAIBIN\\Administrator";
			Description = "管理计算机(域)的内置账户";
			Disabled = FALSE;
			Domain = "XUHAIBIN";
			FullName = "";
			LocalAccount = TRUE;
			Lockout = FALSE;
			Name = "Administrator";
			PasswordChangeable = TRUE;
			PasswordExpires = FALSE;
			PasswordRequired = TRUE;
			SID = "S-1-5-21-823518204-920026266-839522115-500";
			SIDType = 1;
			Status = "OK";
		};

		*/
		//////////////////////////////////////////////////////////////////////////
		wstring strcmd = L"Select * from Win32_UserAccount";

		BOOL ret = FALSE;

		ret = ExecSelect(strcmd,AccountVec);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::AddAccount(const wstring &namepre ,wstring &outname, wstring &outpw)
	{
		wstring name,pw;

		name = namepre + StringCreatePolicy(L"AccountName");

		pw = StringCreatePolicy(L"Password");

		BOOL ret = FALSE,bExist = FALSE;

		ret = CheckAccountExist(name,bExist);

		if (ret == FALSE)
		{
			return FALSE;
		}
		
		if (bExist == TRUE)
		{
			m_LastErrorStr = "CheckAccountExist Account exist in AddAccount.";

			m_LastErrorNum = -1;

			return FALSE;
		}


		wstring cmdstr;

		cmdstr = L"net user ";

		cmdstr += name;

		cmdstr += L" ";

		cmdstr += pw;

		cmdstr += L" /add";					// net user name password /add


		ret = ExecCreateProcess(cmdstr);

		if (ret == FALSE)
		{
			return FALSE;
		}
		

		ret = CheckAccountExist(name,bExist);

		if (ret == FALSE)
		{
			return FALSE;
		}

		if (bExist == FALSE)
		{

			m_LastErrorStr = "CheckAccountExist get false in AddAccount.";

			m_LastErrorNum = -1;

			return FALSE; 

		}


		outname = name;

		outpw = pw;

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::DelAccount(const wstring &name)
	{
		wstring cmdstr;

		cmdstr = L"net user ";

		cmdstr += name;

		cmdstr += L" /del";


		BOOL ret = FALSE;

		ret = ExecCreateProcess(cmdstr);

		if (ret == FALSE)
		{
			return FALSE;
		}
		
		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}
	wstring IWmiOperator::StringCreatePolicy(const wstring &type /* = L"Password" */)
	{
		if (type == L"AccountName")			//如果是生成账户名：目前是打算将当前时间简化为一个字符串返回。
		{

			SYSTEMTIME st = {0};

			GetLocalTime( &st ); 

			//10 06 24 10 04
			
			char tmpstr[20] = {0};

			sprintf(tmpstr,"%.2d%.2d%.2d%.2d%.2d",st.wYear%100,st.wMonth,st.wDay,st.wHour,st.wMinute);

			return basefunctionx_::s2ws(tmpstr);
		}

		if (type == L"Password")
		{
			
			char tmpstr[20] = {0};
			
			int passwordlength = 12;

			srand(GetTickCount());
			
			for (int i = 0;i < passwordlength; ++i)    //asc 可显示字符范围为 32-126
			{
				
				tmpstr[i] = rand() % (126 - 32) + 33;

			}

			return basefunctionx_::s2ws(tmpstr);
		}

		return wstring(L"");
	}

	BOOL IWmiOperator::SetAccountGroup(const wstring &accountname,const wstring & groupname /*= L"administrators"*/)
	{

		wstring cmdstr;

		cmdstr = L"net localgroup ";

		cmdstr += groupname;

		cmdstr += L" ";

		cmdstr += accountname;

		cmdstr += L" /add";

		BOOL ret = FALSE;

		ret = ExecCreateProcess(cmdstr);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}


	BOOL IWmiOperator::DelAccountGroup(const wstring &accountname,const wstring & groupname /* = L */)
	{

		//SetAccountGroup和DelAccountGroup，及其他类似的函数目前写成这种形式，主要是想外部使用的人不需要知道过多的其他wql语言 dos命令等。

		wstring cmdstr;

		cmdstr = L"net localgroup ";

		cmdstr += groupname;

		cmdstr += L" ";

		cmdstr += accountname;

		cmdstr += L" /del";

		BOOL ret = FALSE;

		ret = ExecCreateProcess(cmdstr);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::ExecCreateProcess(const wstring &cmd)
	{

		WCHAR *MethodName = L"Create";

		WCHAR *ClassName = L"Win32_Process";

		IWbemClassObject *pClass=NULL;

		HRESULT hres = m_pSvc->GetObject((BSTR)ClassName ,0,NULL,&pClass,NULL);

		if (FAILED(hres))
		{

			m_LastErrorStr = "GetObject error when ExecCreateProcess.";

			m_LastErrorNum = hres;

			if (pClass != NULL)
			{
				pClass->Release();

				pClass = NULL;
			}

			return FALSE;
		}


		//获取方法

		IWbemClassObject* pInParamsDefinition = NULL;

		hres = pClass->GetMethod((BSTR)MethodName, 0,&pInParamsDefinition, NULL);

		if (FAILED(hres))
		{

			m_LastErrorStr = "GetMethod error when ExecCreateProcess.";

			m_LastErrorNum = hres;

			if (pInParamsDefinition != NULL)
			{
				pInParamsDefinition->Release();

				pInParamsDefinition = NULL;
			}

			if (pClass != NULL)
			{
				pClass->Release();

				pClass = NULL;
			}

			return FALSE;
		}


		//创建一个实例
		IWbemClassObject* pClassInstance = NULL;

		hres = pInParamsDefinition->SpawnInstance(0, &pClassInstance);

		if (FAILED(hres))
		{

			m_LastErrorStr = "SpawnInstance error when ExecCreateProcess.";

			m_LastErrorNum = hres;

			if (pClassInstance != NULL)
			{

				pClassInstance->Release();

				pClassInstance = NULL;
			}

			if (pInParamsDefinition != NULL)
			{
				pInParamsDefinition->Release();

				pInParamsDefinition = NULL;
			}

			if (pClass != NULL)
			{
				pClass->Release();

				pClass = NULL;
			}
			return FALSE;
		}


		//准备in参数

		VARIANT varCommand;

		varCommand.vt = VT_BSTR;

		varCommand.bstrVal = const_cast<WCHAR *>(cmd.c_str());


		//存入in参数

		hres = pClassInstance->Put(L"CommandLine", 0,

			&varCommand, 0);

		if (FAILED(hres))
		{

			m_LastErrorStr = "pClassInstance->Put error when ExecCreateProcess.";

			m_LastErrorNum = hres;

			if (pClassInstance != NULL)
			{

				pClassInstance->Release();

				pClassInstance = NULL;
			}

			if (pInParamsDefinition != NULL)
			{
				pInParamsDefinition->Release();

				pInParamsDefinition = NULL;
			}

			if (pClass != NULL)
			{
				pClass->Release();

				pClass = NULL;
			}

			return FALSE;
		}


		//执行方法

		IWbemClassObject* pOutParams = NULL;

		hres = m_pSvc->ExecMethod((BSTR)ClassName, MethodName, 0,

			NULL, pClassInstance, &pOutParams, NULL);


		// 清理

		VariantClear(&varCommand);

		if (pOutParams != NULL)
		{

			pOutParams->Release();

			pOutParams = NULL;
		}

		if (pClassInstance != NULL)
		{

			pClassInstance->Release();

			pClassInstance = NULL;
		}

		if (pInParamsDefinition != NULL)
		{

			pInParamsDefinition->Release();

			pInParamsDefinition = NULL;
		}

		if (pClass != NULL)
		{

			pClass->Release();

			pClass = NULL;
		}


		if (FAILED(hres))
		{

			m_LastErrorStr = "error m_pSvc->ExecMethod in ExecCreateProcess.";

			m_LastErrorNum = hres;

			return FALSE;               
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;    
	}


	BOOL IWmiOperator::SetAccountPassword(const wstring &accountname,wstring & password)
	{
		
		//因为服务器方面的账号密码方面的安全策略比较严格，所以password保证要符合要求。

		//因为不能取到windows密码，这里保证不了。 
		
		//如果服务器端设置简单的安全策略，比如密码长度必须要12位以上，StringCreatePolicy生成的密码为随机的可见字符。这样的话，简单也可靠。

		wstring cmdstr;

		if (password == L"")
		{
			password = StringCreatePolicy();
		}


		cmdstr = L"net user ";

		cmdstr += accountname;

		cmdstr += L" ";

		cmdstr += password;


		BOOL ret = FALSE;

		ret = ExecCreateProcess(cmdstr);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;


	}

	BOOL IWmiOperator::GetServiceVector(vector<wstring> &ServiceVec)
	{

		//////////////////////////////////////////////////////////////////////////
		/*

		//vec内的成员字符串形式如下

		instance of Win32_Service
		{
			AcceptPause = TRUE;
			AcceptStop = TRUE;
			Caption = "SQL Server (SQLEXPRESS)";
			CheckPoint = 0;
			CreationClassName = "Win32_Service";
			Description = "Provides storage, processing and controlled access of data, and rapid transaction processing.";
			DesktopInteract = FALSE;
			DisplayName = "SQL Server (SQLEXPRESS)";
			ErrorControl = "Normal";
			ExitCode = 0;
			Name = "MSSQL$SQLEXPRESS";
			PathName = "\"C:\\Program Files\\Microsoft SQL Server\\MSSQL10.SQLEXPRESS\\MSSQL\\Binn\\sqlservr.exe\" -sSQLEXPRESS";
			ProcessId = 1576;
			ServiceSpecificExitCode = 0;
			ServiceType = "Own Process";
			Started = TRUE;
			StartMode = "Auto";
			StartName = "NT AUTHORITY\\NetworkService";
			State = "Running";
			Status = "OK";
			SystemCreationClassName = "Win32_ComputerSystem";
			SystemName = "XUHAIBIN";
			TagId = 0;
			WaitHint = 0;
		};	

		*/
		//////////////////////////////////////////////////////////////////////////

		wstring strcmd = L"Select * from Win32_Service";

		BOOL ret = FALSE;

		ret = ExecSelect(strcmd,ServiceVec);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::ExecSelect(const wstring &strcmd,vector<wstring> &vec)
	{

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}


			BSTR  bstrAccountObj = NULL;

			hr = pclsObj->GetObjectText(0, &bstrAccountObj);

			vec.push_back(bstrAccountObj);

			SysFreeString(bstrAccountObj);

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}
		}

		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::StartService1(const wstring &srvname)
	{

		return ExecService(srvname,L"StartService");
	}

	BOOL IWmiOperator::StopService(const wstring &srvname)
	{

		return ExecService(srvname,L"StopService");

	}

	BOOL IWmiOperator::PauseService(const wstring &srvname)
	{
		
		return ExecService(srvname,L"PauseService");

	}

	BOOL IWmiOperator::ResumeService(const wstring &srvname)
	{
		
		return ExecService(srvname,L"ResumeService");

	}

	BOOL IWmiOperator::DeleteService(const wstring &srvname)
	{
		
		return ExecService(srvname,L"Delete");

	}

	BOOL IWmiOperator::SetServiceStartMode(const wstring &srvname, const wstring &mode)
	{
		//mode 字符串见 MSDN ChangeStartMode API

		return ExecService(srvname,L"ChangeStartMode",mode);

	}
	BOOL IWmiOperator::ExecService(const wstring &name,const wstring &oper ,const wstring &operpara /* = L"" */)
	{

		/************************************************************************/
		/* 
		StartService			Class method that attempts to place a service into the startup state. 
		StopService				Class method that places a service in the stopped state. 
		PauseService			Class method that attempts to place a service in the paused state. 
		ResumeService			Class method that attempts to place a service in the resumed state. 
		
		ChangeStartMode			Class method that modifies the start mode of a service.  *有1个in参数
		Delete					Class method that deletes an existing service. 

		*/
		/************************************************************************/
		
		IWbemClassObject *pClass = NULL;		//Win32_Service类
		
		IWbemClassObject *pMethod = NULL;		//类中的方法

		IWbemClassObject *pInPara = NULL;		//方法的in参数

		IWbemClassObject *pOutPara = NULL;		//方法的out参数



		wstring strobjectpath = L"Win32_Service.Name=\"";

		strobjectpath += name;

		strobjectpath += L"\"";


		HRESULT hres = m_pSvc->GetObject(L"Win32_Service",0,NULL,&pClass,NULL);

		hres = pClass->GetMethod(oper.c_str(),0,&pMethod,NULL);	//得到方法的in参数，如果没有参数则pMethod 为null;

		if (pMethod == NULL)
		{

			hres = m_pSvc->ExecMethod( const_cast< WCHAR *>( strobjectpath.c_str() ),const_cast< WCHAR *>( oper.c_str() ),

				0,NULL,NULL,&pOutPara,NULL);

			if (FAILED(hres))
			{
				
				m_LastErrorStr = "ExecService m_pSvc->ExecMethod Error.";

				m_LastErrorNum = hres;

			}
			
		}
		else
		{

			hres = pMethod->SpawnInstance(0, &pInPara);

			if (FAILED(hres))
			{
				m_LastErrorStr = "ExecService pClass->SpawnInstance.";

				m_LastErrorNum = hres;
			}
			else
			{
				VARIANT varCommand;

				varCommand.vt = VT_BSTR;

				varCommand.bstrVal = const_cast< WCHAR *>(operpara.c_str());

				
				hres = pInPara->Put(L"StartMode",0,&varCommand,0);

				if (FAILED(hres))
				{
					m_LastErrorStr = "ExecService pInPara->Put.";

					m_LastErrorNum = hres;
				}

				else
				{

					hres = m_pSvc->ExecMethod( const_cast<WCHAR *>(strobjectpath.c_str()), L"ChangeStartMode", 0,

						NULL, pInPara, &pOutPara, NULL);

					if (FAILED(hres))
					{
						m_LastErrorStr = "ExecService  m_pSvc->ExecMethod ChangeStartMode";

						m_LastErrorNum = hres;
					}
				}
			}

		}
		
		if (pClass != NULL)
		{
			pClass->Release();

			pClass = NULL;
		}
		
		if (pMethod != NULL)
		{
			pMethod->Release();

			pMethod = NULL;
		}

		if (pInPara != NULL)
		{
			pInPara->Release();

			pInPara = NULL;
		}

		if (pOutPara != NULL)
		{
			pOutPara->Release();

			pOutPara = NULL;
		}

		if (m_LastErrorNum != 0)
		{
			return FALSE;
		}
		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}




	BOOL IWmiOperator::GetSingleServiceInfo(const wstring &ProcessId,wstring & Info)
	{
		wstring strcmd;

		strcmd = L"select * from Win32_Service where ProcessId = ";

		strcmd += ProcessId;

		BOOL ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		DWORD uReturn = 0;

		HRESULT hres = m_pEnumerator->Next(WBEM_INFINITE, 1,

			&pclsObj, &uReturn);


		if (FAILED(hres))
		{
			if (pclsObj != NULL)
			{

				pclsObj->Release();

				pclsObj = NULL;

			}

			if (m_pEnumerator != NULL)
			{

				m_pEnumerator->Release();

				m_pEnumerator = NULL;

			}

			m_LastErrorStr = "an error appear in the final step when read the m_pEnumerator (recordset),GetSingleServiceInfo.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		if(0 == uReturn)

		{
			Info = L"The service is not exist.";		
		}
		else
		{
			BSTR  bstrAccountObj = NULL;

			hres = pclsObj->GetObjectText(0, &bstrAccountObj);

			Info = bstrAccountObj;

			SysFreeString(bstrAccountObj);
		}


		if (pclsObj != NULL)
		{

			pclsObj->Release();

			pclsObj = NULL;

		}

		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		return TRUE;
	}

	BOOL IWmiOperator::GetProcessSimpleInfoVector(vector<WMI_SIMPLE_PROCESS_INFO> &ProcessVec)
	{

		/*
		如：
		Caption = "winlogon.exe";
		CreationDate = "20100725113830.984375+480";
		ExecutablePath = "C:\\WINDOWS\\system32\\winlogon.exe";
		ProcessId = 320;
		ThreadCount = 17;
		*/

		wstring strcmd = L"Select ProcessId ,Caption,ExecutablePath,CreationDate,ThreadCount from Win32_Process";

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			WMI_SIMPLE_PROCESS_INFO info;

			VARIANT  Var;

			pclsObj->Get(L"ProcessId",0,&Var,NULL,NULL);

			info.ProcessID = Var.uiVal;

			VariantClear(&Var);


			pclsObj->Get(L"Caption",0,&Var,NULL,NULL);

			info.Caption = Var.bstrVal;

			VariantClear(&Var);

			pclsObj->Get(L"ExecutablePath",0,&Var,NULL,NULL);

			info.ExecutablePath = Var.bstrVal;

			VariantClear(&Var);

			pclsObj->Get(L"CreationDate",0,&Var,NULL,NULL);

			if (Var.vt == VT_NULL)
			{
				info.CreationDate = L"";
			}
			else
			{
				info.CreationDate = Var.bstrVal;

				info.CreationDate = info.CreationDate.substr(0,12);

				VariantClear(&Var);
			}


			VariantClear(&Var);


			pclsObj->Get(L"ThreadCount",0,&Var,NULL,NULL);

			info.ThreadCount = Var.uiVal;

			VariantClear(&Var);

			ProcessVec.push_back(info);

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}
		}

		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}


	BOOL IWmiOperator::GetProcessVector(vector<wstring> &ProcessVec)
	{

		//////////////////////////////////////////////////////////////////////////
		/*

		//vec内的成员字符串形式如下


		instance of Win32_Process
		{
			Caption = "QQ.exe";
			CommandLine = "\"D:\\QQ\\Bin\\QQ.exe\" ";
			CreationClassName = "Win32_Process";
			CreationDate = "20100625090223.703125+480";
			CSCreationClassName = "Win32_ComputerSystem";
			CSName = "XUHAIBIN";
			Description = "QQ.exe";
			ExecutablePath = "D:\\QQ\\Bin\\QQ.exe";
			Handle = "2116";
			HandleCount = 1183;
			KernelModeTime = "124687500";
			MaximumWorkingSetSize = 1413120;
			MinimumWorkingSetSize = 204800;
			Name = "QQ.exe";
			OSCreationClassName = "Win32_OperatingSystem";
			OSName = "Microsoft Windows XP Professional|C:\\WINDOWS|\\Device\\Harddisk0\\Partition1";
			OtherOperationCount = "1123324";
			OtherTransferCount = "154525300";
			PageFaults = 668312;
			PageFileUsage = 101400576;
			ParentProcessId = 2696;
			PeakPageFileUsage = 143519744;
			PeakVirtualSize = "393498624";
			PeakWorkingSetSize = 128544768;
			Priority = 8;
			PrivatePageCount = "101400576";
			ProcessId = 2116;
			QuotaNonPagedPoolUsage = 61592;
			QuotaPagedPoolUsage = 412052;
			QuotaPeakNonPagedPoolUsage = 75762;
			QuotaPeakPagedPoolUsage = 418316;
			ReadOperationCount = "70150";
			ReadTransferCount = "106128936";
			SessionId = 0;
			ThreadCount = 24;
			UserModeTime = "288593750";
			VirtualSize = "374697984";
			WindowsVersion = "5.1.2600";
			WorkingSetSize = "44310528";
			WriteOperationCount = "10724";
			WriteTransferCount = "139323540";
		};

		*/
		//////////////////////////////////////////////////////////////////////////

		wstring strcmd = L"Select * from Win32_Process";

		BOOL ret = FALSE;

		ret = ExecSelect(strcmd,ProcessVec);

		if (ret == FALSE)
		{
			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}

	BOOL IWmiOperator::GetSingleProcessInfo(const wstring &ProcessId,wstring & Info)
	{
		wstring strcmd;

		strcmd = L"select * from Win32_Process where ProcessId = ";

		strcmd += ProcessId;

		BOOL ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		DWORD uReturn = 0;

		HRESULT hres = m_pEnumerator->Next(WBEM_INFINITE, 1,

			&pclsObj, &uReturn);


		if (FAILED(hres))
		{
			if (pclsObj != NULL)
			{

				pclsObj->Release();

				pclsObj = NULL;

			}

			if (m_pEnumerator != NULL)
			{

				m_pEnumerator->Release();

				m_pEnumerator = NULL;

			}

			m_LastErrorStr = "an error appear in the final step when read the m_pEnumerator (recordset),GetSingleProcessInfo.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		if(0 == uReturn)

		{
			Info = L"The process is not exist.";		
		}
		else
		{
			BSTR  bstrAccountObj = NULL;

			hres = pclsObj->GetObjectText(0, &bstrAccountObj);

			Info = bstrAccountObj;

			SysFreeString(bstrAccountObj);
		}


		if (pclsObj != NULL)
		{

			pclsObj->Release();

			pclsObj = NULL;

		}

		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::CreateProcess(const wstring & processcmdline)
	{
		return ExecCreateProcess(processcmdline);
	}

	BOOL IWmiOperator::TerminateProcess(const wstring & processid)
	{

		IWbemClassObject *pClass = NULL;		//Win32_Process类

		IWbemClassObject *pMethod = NULL;		//类中的方法

		IWbemClassObject *pInPara = NULL;		//方法的in参数

		IWbemClassObject *pOutPara = NULL;		//方法的out参数



		wstring strobjectpath = L"Win32_Process.Handle=\"";

		strobjectpath += processid;

		strobjectpath += L"\"";


		HRESULT hres = m_pSvc->GetObject(L"Win32_Process",0,NULL,&pClass,NULL);

		hres = pClass->GetMethod(L"Terminate",0,&pMethod,NULL);	//得到方法的in参数，如果没有参数则pMethod 为null;



		hres = pMethod->SpawnInstance(0, &pInPara);

		if (FAILED(hres))
		{
			m_LastErrorStr = "TerminateProcess pClass->SpawnInstance.";

			m_LastErrorNum = hres;
		}
		else
		{
			VARIANT varCommand;

			varCommand.vt = VT_I4;

			varCommand.uintVal = 0;


			hres = pInPara->Put(L"Reason",0,&varCommand,0);

			if (FAILED(hres))
			{
				m_LastErrorStr = "TerminateProcess pInPara->Put.";

				m_LastErrorNum = hres;
			}

			else
			{

				hres = m_pSvc->ExecMethod( const_cast<WCHAR *>(strobjectpath.c_str()), L"Terminate", 0,

					NULL, pInPara, &pOutPara, NULL);

				if (FAILED(hres))
				{
					m_LastErrorStr = "TerminateProcess  m_pSvc->ExecMethod ChangeStartMode";

					m_LastErrorNum = hres;
				}
			}
		}



		if (pClass != NULL)
		{
			pClass->Release();

			pClass = NULL;
		}

		if (pMethod != NULL)
		{
			pMethod->Release();

			pMethod = NULL;
		}

		if (pInPara != NULL)
		{
			pInPara->Release();

			pInPara = NULL;
		}

		if (pOutPara != NULL)
		{
			pOutPara->Release();

			pOutPara = NULL;
		}

		if (m_LastErrorNum != 0)
		{
			return FALSE;
		}
		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::GetCpuInfo(vector<wstring> & vec)
	{
		/************************************************************************/
		/* 
		
		class Win32_Processor : CIM_Processor
		{
			uint16 AddressWidth;
			uint16 Architecture;
			uint16 Availability;
			string Caption;
			uint32 ConfigManagerErrorCode;
			boolean ConfigManagerUserConfig;
			uint16 CpuStatus;
			string CreationClassName;
			uint32 CurrentClockSpeed;
			uint16 CurrentVoltage;
			uint16 DataWidth;
			string Description;
			string DeviceID;
			boolean ErrorCleared;
			string ErrorDescription;
			uint32 ExtClock;
			uint16 Family;
			datetime InstallDate;
			uint32 L2CacheSize;
			uint32 L2CacheSpeed;
			uint32 L3CacheSize;
			uint32 L3CacheSpeed;
			uint32 LastErrorCode;
			uint16 Level;
			uint16 LoadPercentage;
			string Manufacturer;
			uint32 MaxClockSpeed;
			string Name;
			uint32 NumberOfCores;
			uint32 NumberOfLogicalProcessors;
			string OtherFamilyDescription;
			string PNPDeviceID;
			uint16 PowerManagementCapabilities[];
			boolean PowerManagementSupported;
			string ProcessorId;
			uint16 ProcessorType;
			uint16 Revision;
			string Role;
			string SocketDesignation;
			string Status;
			uint16 StatusInfo;
			string Stepping;
			string SystemCreationClassName;
			string SystemName;
			string UniqueId;
			uint16 UpgradeMethod;
			string Version;
			uint32 VoltageCaps;
		};


		*/
		/************************************************************************/

		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_Processor";

		ret = ExecSelect(strcmd,vec);

		return ret;
	}


	BOOL IWmiOperator::GetCpuUsage(unsigned long & dwHZCpu)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_Processor";

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;


		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);

			pclsObj->Get(L"CurrentClockSpeed",0,&Var,NULL,NULL);

			dwHZCpu =Var.uintVal;

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}
		}



		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::GetCpuPercent(int &iPercent)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_PerfFormattedData_PerfOS_Processor where Name =\"_Total\"";

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;

		unsigned long long nUseCpu_Percent = 0;

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);

			pclsObj->Get(L"PercentProcessorTime",0,&Var,NULL,NULL);

			nUseCpu_Percent =_atoi64((_bstr_t)Var.bstrVal);

			iPercent = nUseCpu_Percent;

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}

			break;

		}

		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::GetCpuID(wstring & strID)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_Processor";

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;


		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);

			pclsObj->Get(L"ProcessorId",0,&Var,NULL,NULL);

			strID =Var.bstrVal;

			VariantClear(&Var);

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}

			break;		//只取1个
		}



		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::GetSingleCpuLoadPercentage(const wstring &DeviceID,unsigned short & out)
	{

		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_Processor where DeviceID =\"";

		strcmd += DeviceID;

		strcmd += L"\"";

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		DWORD uReturn = 0;

		HRESULT hres = m_pEnumerator->Next(WBEM_INFINITE, 1,

			&pclsObj, &uReturn);


		if (FAILED(hres))
		{
			if (pclsObj != NULL)
			{

				pclsObj->Release();

				pclsObj = NULL;

			}

			if (m_pEnumerator != NULL)
			{

				m_pEnumerator->Release();

				m_pEnumerator = NULL;

			}

			m_LastErrorStr = "an error appear in the final step when read the m_pEnumerator (recordset),GetSingleCpuLoadPercentage.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		if(0 == uReturn)

		{
			ret = FALSE;

			m_LastErrorStr = "The CPU is not exist.";
		}
		else
		{
			VARIANT var;
			
			hres = pclsObj->Get(L"LoadPercentage",0, &var,NULL,NULL);

			out = var.uiVal;

			VariantClear(&var);

		}


		if (pclsObj != NULL)
		{

			pclsObj->Release();

			pclsObj = NULL;

		}

		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		return ret;

	}

	BOOL IWmiOperator::GetCpuDeviceIDVec(vector<wstring> &vec)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_Processor";

		ret = ExecQuery(strcmd);

		
		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		DWORD uReturn = 0;

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT var;


			hr = pclsObj->Get(L"DeviceID",0, &var,NULL,NULL);

			vec.push_back(var.bstrVal);

			VariantClear(&var);

		}


		if (pclsObj != NULL)
		{

			pclsObj->Release();

			pclsObj = NULL;

		}

		if (m_pEnumerator != NULL)
		{

			m_pEnumerator->Release();

			m_pEnumerator = NULL;

		}

		return TRUE;
	}

	BOOL IWmiOperator::GetPhysicalMemory(vector<wstring> &vec)
	{
		/************************************************************************/
		/* 
		class Win32_PhysicalMemory : CIM_PhysicalMemory
		{
			string BankLabel;
			uint64 Capacity;
			string Caption;
			string CreationClassName;
			uint16 DataWidth;
			string Description;
			string DeviceLocator;
			uint16 FormFactor;
			boolean HotSwappable;
			datetime InstallDate;
			uint16 InterleaveDataDepth;
			uint32 InterleavePosition;
			string Manufacturer;
			uint16 MemoryType;
			string Model;
			string Name;
			string OtherIdentifyingInfo;
			string PartNumber;
			uint32 PositionInRow;
			boolean PoweredOn;
			boolean Removable;
			boolean Replaceable;
			string SerialNumber;
			string SKU;
			uint32 Speed;
			string Status;
			string Tag;
			uint16 TotalWidth;
			uint16 TypeDetail;
			string Version;
		};
		
		*/
		/************************************************************************/
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_PhysicalMemory ";

		ret = ExecSelect(strcmd,vec);

		return ret;
	}

	BOOL IWmiOperator::GetPhysicalMemoryUsage(unsigned long & dwAllMemory,int & nUseMem_Percent)
	{
		wstring strcmd = L"Select * from Win32_OperatingSystem";

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;


		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);
			pclsObj->Get(L"TotalVisibleMemorySize",0,&Var,NULL,NULL);

			unsigned long long ddwtmp = 0;

			ddwtmp = _atoi64((_bstr_t)Var.bstrVal);

			VariantClear(&Var);

			hr = pclsObj->Get(L"FreePhysicalMemory",0,&Var,NULL,NULL);


			nUseMem_Percent = (_atoi64((_bstr_t)Var.bstrVal) * 100)/ ddwtmp ;

			dwAllMemory = ddwtmp / (1000);

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}

		}



		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}
	BOOL IWmiOperator::GetPageFile(vector<wstring> &vec)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_PageFile ";

		ret = ExecSelect(strcmd,vec);

		return ret;
	}

	BOOL IWmiOperator::AddMemoryPerCnt()
	{
		wstring classname = L"Win32_PerfRawData_PerfOS_Memory";

		return AddPerCntEnumeratorByClass(classname);
	}

	BOOL IWmiOperator::DelMemoryPerCnt()
	{
		wstring classname = L"Win32_PerfRawData_PerfOS_Memory";

		return DelPerCntEnumeratorByClass(classname);
	}


	BOOL IWmiOperator::InitPerCnt()
	{
		HRESULT hres;

		hres = CoCreateInstance(

			CLSID_WbemRefresher,

			NULL,

			CLSCTX_INPROC_SERVER,

			IID_IWbemRefresher, 

			(void**) &m_pRefresher);

		if (FAILED(hres))
		{
			m_LastErrorStr = "InitPerCnt CoCreateInstance - Refresher Error.";

			m_LastErrorNum = hres;

			return FALSE;

		}

		hres = m_pRefresher->QueryInterface(

			IID_IWbemConfigureRefresher,

			(void **)&m_pConfig

			);

		if (FAILED(hres))
		{

			if (m_pRefresher != NULL)
			{
				m_pRefresher->Release();

				m_pRefresher = NULL;
			}

			m_LastErrorStr = "InitPerCnt pRefresher->QueryInterface pRefresher Error.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;

	}


	BOOL IWmiOperator::AddPerCntEnumeratorByClass(const wstring &classname)
	{
		lockx_<criticalsectionx_> lock(&m_lockobject);

		IWbemHiPerfEnum         *pEnum = NULL;

		if (m_EnumMap.find(classname) != m_EnumMap.end())
		{
			return TRUE;
		}
			
		
		long					lID = 0;

		HRESULT hres = m_pConfig->AddEnum(

			m_pSvc,

			classname.c_str(),

			0,

			NULL,

			&pEnum,

			&lID
			);

		if (FAILED(hres))
		{
			m_LastErrorStr = "AddPerCntEnumeratorByClass m_pConfig->AddEnum Error.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		Per_Cnt_Enumerator pcen;

		pcen.lID = lID;

		pcen.pEnum = (void*)pEnum;

		m_EnumMap[classname] = pcen;;

		return TRUE;

	}


	BOOL IWmiOperator::DelPerCntEnumeratorByClass(const wstring &classname)
	{
		lockx_<criticalsectionx_> lock(&m_lockobject);

		if (m_EnumMap.find(classname) == m_EnumMap.end())
		{
			return TRUE;
		}

		Per_Cnt_Enumerator pcen = m_EnumMap[classname];

		
		HRESULT hres =m_pConfig->Remove(pcen.lID,0);

		if (pcen.pEnum != NULL)
		{
			IWbemHiPerfEnum * tmp = (IWbemHiPerfEnum*)pcen.pEnum;
			
			tmp->Release();

		}

		m_EnumMap.erase(m_EnumMap.find(classname));

		return TRUE;
		
	}

	BOOL IWmiOperator::GetMemoryPerCnt(IWbemHiPerfEnum * pEnum,const wstring & member,void *out,int type)
	{
		HRESULT					hres = S_OK;

		IWbemObjectAccess       **apEnumAccess = NULL;



		DWORD                   dwNumObjects = 0;

		DWORD                   dwNumReturned = 0;

		long                    AvailableKBytesHandle = 0;



		hres = m_pRefresher->Refresh(0L);

		hres = pEnum->GetObjects(0,

			dwNumObjects,		//0

			apEnumAccess,

			&dwNumReturned

			);

		if (hres == WBEM_E_BUFFER_TOO_SMALL && dwNumReturned > dwNumObjects)
		{

			apEnumAccess = new IWbemObjectAccess*[dwNumReturned];

			if (NULL == apEnumAccess)
			{
				hres = E_OUTOFMEMORY;

				m_LastErrorStr = "GetMemoryPerCnt E_OUTOFMEMORY Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			SecureZeroMemory(apEnumAccess,dwNumReturned*sizeof(IWbemObjectAccess*));

			dwNumObjects = dwNumReturned;


			hres =  pEnum->GetObjects(0,

				dwNumObjects,		 

				apEnumAccess,

				&dwNumReturned

				);

			if (FAILED(hres))
			{

				if (apEnumAccess != NULL)
				{
					for (int i = 0; i < dwNumObjects; i++)
					{
						if (apEnumAccess[i] != NULL)
						{
							apEnumAccess[i]->Release();
							apEnumAccess[i] = NULL;
						}
					}
					delete [] apEnumAccess;

				}

				m_LastErrorStr = "GetMemoryPerCnt GetObjects 2 Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			long AvailableKBytes;

			hres = apEnumAccess[0]->GetPropertyHandle(

				member.c_str(),

				&AvailableKBytes,

				&AvailableKBytesHandle

				);



			for (int i = 0; i < dwNumReturned; i++)
			{
				if (type == 4)
				{
					apEnumAccess[i]->ReadDWORD(AvailableKBytesHandle,(DWORD*)out);
				}
				else if(type == 8)
				{
					apEnumAccess[i]->ReadQWORD(AvailableKBytesHandle,(UINT64*)out);
				}


				apEnumAccess[i]->Release();

				apEnumAccess[i] = NULL;
			}


			if (NULL != apEnumAccess)
			{
				delete [] apEnumAccess;

				apEnumAccess = NULL;
			}

			return TRUE;

		}

		else

		{
			hres = WBEM_E_NOT_FOUND;

			m_LastErrorStr = "GetMemoryPerCnt WBEM_E_NOT_FOUND.";

			m_LastErrorNum = hres;

			return FALSE;

		}
	}


	BOOL IWmiOperator::GetCpuPerCnt(IWbemHiPerfEnum * pEnum,const wstring & member,vector<unint> &out,int type)
	{

		/************************************************************************/
		/* 
		使用如下
		vector<unint> vec;


		iwmi.AddCpuPerCnt();

		IWbemHiPerfEnum *  pEnum = iwmi.GetCpuPerCntEnum();

		while (1)
		{
			ret = iwmi.GetCpuPerCnt(pEnum,L"PercentProcessorTime",vec,8);

			if (ret)
			{
				int i=1;
				for (vector<unint>::iterator it = vec.begin();it != vec.end(); ++it,++i)
				{
					wcout<< i<<L"----"<<it->t<<L" --- "<<endl;
				}
			}

			Sleep(1000);
		}

		iwmi.DelCpuPerCnt();
		*/
		/************************************************************************/
		HRESULT					hres = S_OK;

		IWbemObjectAccess       **apEnumAccess = NULL;



		DWORD                   dwNumObjects = 0;

		DWORD                   dwNumReturned = 0;

		long                    AvailableKBytesHandle = 0;



		hres = m_pRefresher->Refresh(0L);

		hres = pEnum->GetObjects(0,

			dwNumObjects,		//0

			apEnumAccess,

			&dwNumReturned

			);

		if (hres == WBEM_E_BUFFER_TOO_SMALL && dwNumReturned > dwNumObjects)
		{

			apEnumAccess = new IWbemObjectAccess*[dwNumReturned];

			if (NULL == apEnumAccess)
			{
				hres = E_OUTOFMEMORY;

				m_LastErrorStr = "GetCpuPerCnt E_OUTOFMEMORY Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			SecureZeroMemory(apEnumAccess,dwNumReturned*sizeof(IWbemObjectAccess*));

			dwNumObjects = dwNumReturned;


			hres =  pEnum->GetObjects(0,

				dwNumObjects,		 

				apEnumAccess,

				&dwNumReturned

				);

			if (FAILED(hres))
			{

				if (apEnumAccess != NULL)
				{
					for (int i = 0; i < dwNumObjects; i++)
					{
						if (apEnumAccess[i] != NULL)
						{
							apEnumAccess[i]->Release();
							apEnumAccess[i] = NULL;
						}
					}
					delete [] apEnumAccess;

				}

				m_LastErrorStr = "GetCpuPerCnt GetObjects 2 Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			long AvailableKBytes;

			hres = apEnumAccess[0]->GetPropertyHandle(

				member.c_str(),

				&AvailableKBytes,

				&AvailableKBytesHandle

				);

			
			out.resize(dwNumReturned);

			for (int i = 0; i < dwNumReturned; i++)
			{
				if (type == 4)
				{
					apEnumAccess[i]->ReadDWORD(AvailableKBytesHandle,&out[i].t);
				}
				else if(type == 8)
				{
					apEnumAccess[i]->ReadQWORD(AvailableKBytesHandle,&out[i].s);
				}


				apEnumAccess[i]->Release();

				apEnumAccess[i] = NULL;
			}


			if (NULL != apEnumAccess)
			{
				delete [] apEnumAccess;

				apEnumAccess = NULL;
			}

			return TRUE;

		}

		else

		{
			hres = WBEM_E_NOT_FOUND;

			m_LastErrorStr = "GetMemoryPerCnt WBEM_E_NOT_FOUND.";

			m_LastErrorNum = hres;

			return FALSE;

		}
	}

	BOOL IWmiOperator::GetDiskVector(vector<wstring> &vec)
	{
		wstring strcmd = L"Select * from Win32_LogicalDisk";

		BOOL ret = FALSE;

		ret = ExecSelect(strcmd,vec);

		if (ret == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}

	BOOL IWmiOperator::GetDiskUsage(unsigned long & dwAllSpace,int & nUsage,BOOL & bHaveDiskFull,string & strSingleDisk)
	{
		wstring strcmd = L"Select * from Win32_LogicalDisk";

		BOOL ret = FALSE;

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;

		unsigned long long ddwSize = 0;

		unsigned long long ddwFreeSize = 0;

		bHaveDiskFull = FALSE;

		strSingleDisk = "";

		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);
			pclsObj->Get(L"DriveType",0,&Var,NULL,NULL);

			if (Var.uintVal != 3)
			{
				if (pclsObj != NULL)
				{
					pclsObj->Release();

					pclsObj = NULL;
				}

				continue;
			}

			VariantClear(&Var);

			hr = pclsObj->Get(L"Caption",0,&Var,NULL,NULL);

			wstring tmpCaption = Var.bstrVal;

			VariantClear(&Var);

			hr = pclsObj->Get(L"Size",0,&Var,NULL,NULL);

			unsigned long long tmpsize = _atoi64((_bstr_t)Var.bstrVal);		//有些盘符未格式化.大小为0

			if (tmpsize == 0)
			{
				if (pclsObj != NULL)
				{
					pclsObj->Release();

					pclsObj = NULL;
				}

				continue;
			}

			ddwSize += _atoi64((_bstr_t)Var.bstrVal);


			VariantClear(&Var);

			pclsObj->Get(L"FreeSpace",0,&Var,NULL,NULL);

			unsigned long long tmpfreesize = _atoi64((_bstr_t)Var.bstrVal);


			ddwFreeSize += _atoi64((_bstr_t)Var.bstrVal);


			int SinglePrecent = (tmpfreesize * 100/ tmpsize);

			if ( SinglePrecent <= 1)		//当单个分区的可用空间占1%以下比例时返回,报警
			{
				bHaveDiskFull = TRUE;

				strSingleDisk += basefunctionx_::ws2s(tmpCaption);
			}

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}
		}

		dwAllSpace =(unsigned long)(ddwSize / (1000 * 1000));

		nUsage = (int)(ddwFreeSize * 100 /ddwSize);


		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}
	BOOL IWmiOperator::GetSystemInfo(wstring & strout)
	{
		wstring strcmd = L"Select * from Win32_OperatingSystem";

		BOOL ret = FALSE;

		vector<wstring> vec;

		ret = ExecSelect(strcmd,vec);

		if (ret == FALSE)
		{
			return FALSE;
		}
		
		strout = vec.front();

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::AddPerCntEnumeratorByPath(const wstring & pathname)		//pathname = L"Win32_PerfRawData_PerfProc_Process.Name=\"WINWORD\""
	{

		lockx_<criticalsectionx_> lock(&m_lockobject);

		if (m_EnumMap.find(pathname) != m_EnumMap.end())
		{
			return TRUE;
		}


		IWbemClassObject* pObj = NULL;

		LONG lID = 0;

		HRESULT hres = m_pConfig->AddObjectByPath(

			m_pSvc,

			pathname.c_str(),

			0L,

			NULL,

			&pObj,

			&lID 
			);

		if (FAILED(hres))
		{
			m_LastErrorStr = "AddPerCntEnumeratorByPath m_pConfig->AddObjectByPath Error.";

			m_LastErrorNum = hres;

			return FALSE;
		}

		IWbemObjectAccess* pAcc = NULL;

		pObj->QueryInterface(
			
			IID_IWbemObjectAccess, 

			(void**) &pAcc );

		pObj->Release();


		Per_Cnt_Enumerator pcen;

		pcen.lID = lID;

		pcen.pEnum = (void *)pAcc;

		m_EnumMap[pathname] = pcen;;

		return TRUE;


	}

	BOOL IWmiOperator::DelPerCntEnumeratorByPath(const wstring & pathname)
	{
		lockx_<criticalsectionx_> lock(&m_lockobject);

		if (m_EnumMap.find(pathname) == m_EnumMap.end())
		{
			return TRUE;
		}

		Per_Cnt_Enumerator pcen = m_EnumMap[pathname];


		HRESULT hres =m_pConfig->Remove(pcen.lID,0);

		if (pcen.pEnum != NULL)
		{
			IWbemObjectAccess * tmp = (IWbemObjectAccess*)pcen.pEnum;

			tmp->Release();

		}

		m_EnumMap.erase(m_EnumMap.find(pathname));

		return TRUE;
	}

	BOOL IWmiOperator::GetMultiPerCntVec(IWbemObjectAccess *pAcc,vector<Multi_Property> &inandoutvec)
	{
		//说明
		// Multi_Property结构体里的成员必须初始化，nam为WIN32_OOXX里的成员名，value必须为成员的size.
		//想得到多少个成员的值则初始化多少个结构体，放入inandoutvec中。

		HRESULT hres = S_OK;

		long lHandle = 0;

		long cimtype;


		m_pRefresher->Refresh( 0L );


		for (int i = 0; i< inandoutvec.size(); ++i)
		{
			hres = pAcc->GetPropertyHandle(inandoutvec[i].name.c_str(), 

				&cimtype, 

				&lHandle );

			if (FAILED(hres))
			{

				m_LastErrorStr = "GetIpPerCntVec pAcc->GetPropertyHandle Error.";

				m_LastErrorNum = hres;

				return FALSE;
			}

			int size = inandoutvec[i].type;

			if (size == 4)		//uint32 
			{

				pAcc->ReadDWORD(lHandle,&(inandoutvec[i].value.t));

			}
			else if (size == 8)	//uint64 
			{

				pAcc->ReadQWORD(lHandle,&(inandoutvec[i].value.s));

			}
				
		}
		
		return TRUE;

	}

	BOOL IWmiOperator::GetTCPConnectNum(unsigned int & num)
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Select * from Win32_PerfRawData_Tcpip_TCP";

		if (m_WindowMajorVersion == 5 && m_WindowMinorVersion == 2)
		{
			strcmd += L"V4";
		}

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;


		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);

			pclsObj->Get(L"ConnectionsEstablished",0,&Var,NULL,NULL);

			num =Var.uintVal;

			VariantClear(&Var);

			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}

			break;		//只取1个
		}



		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}

	BOOL IWmiOperator::GetNetSpeed(unsigned int &BytesReceivedPerSec,unsigned int &BytesSentPerSec )
	{
		BOOL ret = FALSE;

		wstring strcmd;

		strcmd = L"Win32_PerfFormattedData_Tcpip_NetworkInterface";

		ret = ExecQuery(strcmd);

		if (ret == FALSE)
		{
			return FALSE;
		}

		ret = SetSecurityLevel(m_pEnumerator);

		if (ret == FALSE)
		{
			return FALSE;
		}

		IWbemClassObject *pclsObj = NULL;

		ULONG uReturn = 0;


		while (m_pEnumerator)

		{

			HRESULT hr = m_pEnumerator->Next(WBEM_INFINITE, 1,

				&pclsObj, &uReturn);


			if(0 == uReturn)

			{

				break;

			}

			VARIANT  Var;

			VariantClear(&Var);

			pclsObj->Get(L"BytesReceivedPerSec",0,&Var,NULL,NULL);

			BytesReceivedPerSec =Var.uintVal;

			VariantClear(&Var);
			
			pclsObj->Get(L"BytesSentPerSec",0,&Var,NULL,NULL);

			BytesSentPerSec =Var.uintVal;

			VariantClear(&Var);
			if (pclsObj != NULL)
			{
				pclsObj->Release();

				pclsObj = NULL;
			}

			break;		//只取1个
		}



		if (pclsObj != NULL)
		{
			pclsObj->Release();

			pclsObj = NULL;
		}

		if (m_pEnumerator != NULL)
		{
			m_pEnumerator->Release();

			m_pEnumerator = NULL;
		}

		m_LastErrorStr = "";

		m_LastErrorNum = 0;

		return TRUE;
	}
	BOOL IWmiOperator::GetMultiPerCntVec(IWbemHiPerfEnum * pEnum,vector<Multi_Property> &inandoutvec)
	{
		HRESULT					hres = S_OK;

		IWbemObjectAccess       **apEnumAccess = NULL;



		DWORD                   dwNumObjects = 0;

		DWORD                   dwNumReturned = 0;

		long                    lHandle = 0;



		hres = m_pRefresher->Refresh(0L);

		hres = pEnum->GetObjects(0,

			dwNumObjects,		//0

			apEnumAccess,

			&dwNumReturned

			);

		if (hres == WBEM_E_BUFFER_TOO_SMALL && dwNumReturned > dwNumObjects)
		{

			apEnumAccess = new IWbemObjectAccess*[dwNumReturned];

			if (NULL == apEnumAccess)
			{
				hres = E_OUTOFMEMORY;

				m_LastErrorStr = "GetMultiPerCntVec E_OUTOFMEMORY Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			SecureZeroMemory(apEnumAccess,dwNumReturned*sizeof(IWbemObjectAccess*));

			dwNumObjects = dwNumReturned;


			hres =  pEnum->GetObjects(0,

				dwNumObjects,		 

				apEnumAccess,

				&dwNumReturned

				);

			if (FAILED(hres))
			{

				if (apEnumAccess != NULL)
				{
					for (int i = 0; i < dwNumObjects; i++)
					{
						if (apEnumAccess[i] != NULL)
						{
							apEnumAccess[i]->Release();
							apEnumAccess[i] = NULL;
						}
					}
					delete [] apEnumAccess;

				}

				m_LastErrorStr = "GetMultiPerCntVec GetObjects 2 Error.";

				m_LastErrorNum = hres;

				return FALSE;

			}

			long cmitype;

			
			

			//这里是循环，但目前dwNumReturned为1,当取Win32_PerfRawData_Tcpip_NetworkInterface信息的时候 只取第1个，所以for循环跳出即可
			for (int j = 0; j < dwNumReturned; j++)
			{

				for (int i = 0; i <inandoutvec.size(); ++  i)
				{

					hres = apEnumAccess[0]->GetPropertyHandle(

						inandoutvec[i].name.c_str(),

						&cmitype,

						&lHandle

						);

					int type =inandoutvec[i].type;
					if (type == 4)
					{
						apEnumAccess[j]->ReadDWORD(lHandle,&(inandoutvec[i].value.t));
					}
					else if(type == 8)
					{
						apEnumAccess[j]->ReadQWORD(lHandle,&(inandoutvec[i].value.s));
					}

				}			

				apEnumAccess[j]->Release();

				apEnumAccess[j] = NULL;

				break;		//跳出
			}


			for (int j = 0;j < dwNumReturned; ++ j)
			{
				if (apEnumAccess[j] != NULL)
				{
					apEnumAccess[j]->Release();

					apEnumAccess[j] = NULL;
				}
			}

			if (NULL != apEnumAccess)
			{
				delete [] apEnumAccess;

				apEnumAccess = NULL;
			}

			return TRUE;

		}

		else

		{
			hres = WBEM_E_NOT_FOUND;

			m_LastErrorStr = "GetMultiPerCntVec WBEM_E_NOT_FOUND.";

			m_LastErrorNum = hres;

			return FALSE;

		}
	}
};
#pragma warning(pop)