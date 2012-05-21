#include "common.h"

string getipbyint(unsigned int uiip)
{
	in_addr in={0};
	in.s_addr = uiip;
	return inet_ntoa(in);
}

unsigned int getipbystring( string strip )
{
	return inet_addr(strip.c_str());
}
unsigned int getticktime()
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

int g_iloglv = 5;
workthread*			g_workthread = NULL;
buffqueuex<1000,false,true>		g_sendmsgqueue;	

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	return TRUE;
}
