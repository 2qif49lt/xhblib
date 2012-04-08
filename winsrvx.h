#ifndef WIN_SERVICEX_XHB_H_
#define WIN_SERVICEX_XHB_H_


#include <windows.h>
#include <winsvc.h>

class winsrvx
{
public:
	static int start(int argc,char** argv);
private:
	winsrvx();
	~winsrvx(){}

public:
	static bool installsrv();
	static bool uninstallsrv();
	static bool runsrv();

	static char* szsrvname ;
	static char* szsrvdisplayname;
	static char* szsrvdescription;

	static SERVICE_STATUS srvstatus;
	static SERVICE_STATUS_HANDLE srvstatushandle; 
	static HANDLE hsrvrun;

private:
	static void WINAPI srvmain(DWORD argc, char **argv);
	static void WINAPI schandler(DWORD dwcode);
	static bool _initrun();
	static void _exitinit();
	static int _run();	

};


#endif	//WIN_SERVICE_XHB_H_