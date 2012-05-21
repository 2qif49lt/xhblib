#ifndef COMMON_LOGSRV_XHB_H_
#define COMMON_LOGSRV_XHB_H_

#define REMOTELOG_DLL_IMPLEMENT_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#pragma comment(lib,"ws2_32.lib")

#include "winsock2.h"

#pragma warning(disable:4996)
#pragma warning(disable:4200)
#else

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string>
using namespace std;
#include "msg.h"

#include "mthread.h"
#include "workthread.h"
#include "buffqueue.h"

//getipbyint transform 16777343 to "127.0.0.1"
extern string getipbyint(unsigned int uiip);
//getipbystring transform "127.0.0.1" to 16777343
extern unsigned int getipbystring( string strip );

extern unsigned int getinterval(unsigned int uilast,unsigned int uinow);
extern unsigned int getticktime();

extern int g_iloglv;
class workthread;
extern workthread*			g_workthread;
extern buffqueuex<1000,false,true>		g_sendmsgqueue;	


#endif //COMMON_LOGSRV_XHB_H_