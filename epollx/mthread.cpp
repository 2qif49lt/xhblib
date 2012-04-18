#include "mthread.h"
#ifdef _WIN32
#pragma warning(disable:4312)
#endif

bool mthread::initinstance()
{
	//do prepare work;
	return true;
}
void mthread::mainfunction()
{
	//demo function
	int i = 1;
	while(brun)
	{
		//do things
		printf("%d\n",++i);
	}
}
unsigned int mthread::exitinstance()
{
	//do exit thing
    brun = false;
	return 0;
}
unsigned int mthread::run()
{
	if (!initinstance())
	{
		exitinstance();
		return 1;
	}	
	mainfunction();
	exitinstance();
	return 0;
}
#ifdef _WIN32
DWORD WINAPI mthread::threadfunction(void* param)
{
    mthread* t = (mthread*)param;
    unsigned int uiret = t->run();
    return (DWORD)uiret;
}
#else
void* mthread::threadfunction(void* param)
{
    mthread* t = (mthread*)param;
    unsigned int uiret = t->run();
    return (void*)uiret;
}
#endif
bool mthread::start()
{
#ifdef _WIN32
    DWORD dwthreadid = 0;
    HANDLE hThread = NULL;
    hThread = CreateThread( 
        NULL,              // default security attributes
        0,                 // use default stack size  
        mthread::threadfunction,          // thread function 
        (void*)this,             // argument to thread function 
        0,                 // use default creation flags 
        &dwthreadid);   // returns the thread identifier 

    if (hThread == NULL)
    {
        return false;
    }
    CloseHandle(hThread);
    tid = dwthreadid;
#else
    pthread_t t;

    int nret = pthread_create(&t,NULL,&threadfunction,(void*)this);
    if (nret != 0)
    {
        return false;
    }
    tid = *(unsigned long*)(&t);
#endif
    return true;
}
void mthread::millisleep(unsigned int uimillisecond)
{
#ifdef _WIN32
	Sleep(uimillisecond);
#else
	usleep(uimillisecond * 1000);
#endif
}
