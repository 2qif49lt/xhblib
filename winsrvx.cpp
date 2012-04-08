#include "winsrvx.h"
#include <algorithm>
#include <iostream>
#include<fstream>
#include <string>

using std::string;
using std::ofstream;
using std::cout;
using std::endl;
using std::transform;


char* winsrvx::szsrvname = "Xtestsrvname";
char* winsrvx::szsrvdisplayname = "Xtestsrvdisplayname";
char* winsrvx::szsrvdescription = "Xtest srv description";
SERVICE_STATUS winsrvx::srvstatus = {0};
SERVICE_STATUS_HANDLE winsrvx::srvstatushandle = 0; 
HANDLE winsrvx::hsrvrun = 0;

winsrvx::winsrvx(){}

bool winsrvx::installsrv()
{
	SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);

	if (!scm)
	{
		return false;
	}

	char szpath[MAX_PATH + 10] = {};
	szpath[0] = '\"';

	if (0 == GetModuleFileName( 0, szpath + 1 , sizeof(szpath) - 1))
	{
		CloseServiceHandle(scm);
		return false;
	}
	size_t ilen = strlen(szpath);
	szpath[ilen] = '\"';
	szpath[ilen + 1] = '\0';

	strcat(szpath, " -run");

	SC_HANDLE srv = CreateService(scm,
		szsrvname,						// name of service
		szsrvdisplayname,				// service name to display
		SERVICE_ALL_ACCESS,                         // desired access
		// service type
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,                         // start type
		SERVICE_ERROR_IGNORE,                       // error control type
		szpath,										// service's binary
		0,                                          // no load ordering group
		0,                                          // no tag identifier
		0,                                          // no dependencies
		0,                                          // LocalSystem account
		0);                                         // no password

	if (!srv)
	{
		CloseServiceHandle(scm);
		return false;
	}

	SERVICE_DESCRIPTION srvdesc;
	srvdesc.lpDescription = szsrvdescription;
	ChangeServiceConfig2(
		srv,
		SERVICE_CONFIG_DESCRIPTION,  
		&srvdesc);      

	SC_ACTION action;
	action.Type = SC_ACTION_RESTART;
	action.Delay = 10000;
	SERVICE_FAILURE_ACTIONS sfa;
	ZeroMemory(&sfa, sizeof(SERVICE_FAILURE_ACTIONS));
	sfa.lpsaActions = &action;
	sfa.cActions = 1;
	sfa.dwResetPeriod =INFINITE;
	ChangeServiceConfig2(
		srv,                                // handle to service
		SERVICE_CONFIG_FAILURE_ACTIONS,         // information level
		&sfa);                                  // new data

	CloseServiceHandle(srv);
	CloseServiceHandle(scm);
	return true;
}

bool winsrvx::uninstallsrv()
{
	SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

	if (!scm)
	{
		return false;
	}

	SC_HANDLE srv = OpenService(scm,
		szsrvname, SERVICE_QUERY_STATUS | DELETE);

	if (!srv)
	{
		CloseServiceHandle(scm);
		return false;
	}

	SERVICE_STATUS ss;
	if (QueryServiceStatus(srv, &ss))
	{
		if (ss.dwCurrentState == SERVICE_STOPPED)
			DeleteService(srv);
	}

	CloseServiceHandle(srv);
	CloseServiceHandle(scm);
	return true;
}

bool winsrvx::runsrv()
{
	SERVICE_TABLE_ENTRY srvtable[] =
	{
		{ szsrvname,&winsrvx::srvmain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(srvtable))
	{
		return false;
	}
	return true;
}
void WINAPI winsrvx::schandler(DWORD dwcode)
{
	switch (dwcode)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;

	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		srvstatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(srvstatushandle, &srvstatus);
		SetEvent(hsrvrun);
		return;

	case SERVICE_CONTROL_PAUSE:
		srvstatus.dwCurrentState = SERVICE_PAUSED;
		SetServiceStatus(srvstatushandle, &srvstatus);
		break;

	case SERVICE_CONTROL_CONTINUE:
		srvstatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(srvstatushandle, &srvstatus);
		break;

	default:
		if ( dwcode >= 128 && dwcode <= 255 )
			// user defined control code
			break;
		else
			// unrecognized control code
			break;
	}

	SetServiceStatus(srvstatushandle, &srvstatus);
}
void WINAPI winsrvx::srvmain(DWORD argc, char **argv)
{
	srvstatus.dwServiceType = SERVICE_WIN32;
	srvstatus.dwCurrentState = SERVICE_START_PENDING;
	srvstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP ;
	srvstatus.dwWin32ExitCode = NO_ERROR;
	srvstatus.dwServiceSpecificExitCode = NO_ERROR;
	srvstatus.dwCheckPoint = 0;
	srvstatus.dwWaitHint = 0;

	srvstatushandle = 0;
	srvstatushandle = RegisterServiceCtrlHandler(szsrvname, winsrvx::schandler);

	if ( srvstatushandle )
	{
		char szpath[MAX_PATH] = {};

		GetModuleFileName(0, szpath, sizeof(szpath));

		for (size_t i = strlen(szpath) - 1; i >= 0 ; --i)
		{
			if (szpath[i] == '\\') 
			{
				szpath[i] = '\0';
				break;
			}
		}


		// service is starting
		srvstatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus(srvstatushandle, &srvstatus);

		// do initialisation here
		SetCurrentDirectory(szpath);

		// running
		srvstatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		srvstatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( srvstatushandle, &srvstatus );

		////////////////////////
		// service main cycle //
		////////////////////////

		_initrun();
		_run();
		_exitinit();
		
		// service was stopped
		srvstatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(srvstatushandle, &srvstatus);

		// do cleanup here

		// service is now stopped
		srvstatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		srvstatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(srvstatushandle, &srvstatus);
	}
}

bool winsrvx::_initrun()
{
	if(!hsrvrun)
		hsrvrun = CreateEvent(NULL,FALSE,FALSE,NULL);
	return hsrvrun != NULL;
}
void winsrvx::_exitinit()
{
	if(hsrvrun) 
		CloseHandle(hsrvrun);
}
int winsrvx::_run()
{
	DWORD dwlasttick = GetTickCount();

	while(WaitForSingleObject(hsrvrun,10) == WAIT_TIMEOUT)
	{
		if(GetTickCount() - dwlasttick >= 1000)
		{
			cout<<GetTickCount()<<endl;
			dwlasttick = GetTickCount();
		}
	}
	return 0;
}

int winsrvx::start(int argc,char** argv)
{
	char chmode = 'd';
	if(argc > 1)
	{
		string strpara = argv[1];
		transform(strpara.begin(),strpara.end(),strpara.begin(),tolower);
		if(strpara == "-i" || strpara == "-install")
			chmode = 'i';
		if(strpara == "-u" || strpara == "-uninstall")
			chmode = 'u';
		if(strpara == "-r" || strpara == "-run")	//scm
			chmode = 'r';
	}
	int iret = 0;
	switch (chmode)
	{
	case 'i':
		winsrvx::installsrv();
		return 0;
	case 'u':
		winsrvx::uninstallsrv();
		return 0;
	case 'r':
		winsrvx::runsrv();
		return 0;
	case 'd':
		{
			winsrvx::_initrun();
			iret = winsrvx::_run();
			winsrvx::_exitinit();
		}
		break;
	}
	return iret;
}
int main(int argc, char ** argv)
{
	return winsrvx::start(argc,argv);
}
