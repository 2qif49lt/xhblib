#ifndef THREAD_MULTI_PLAT_H_
#define THREAD_MULTI_PLAT_H_

//windows

#ifdef _WIN32
#include "windows.h"
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include <stdio.h>


class mthread
{
public:
	mthread():brun(true),tid(0){}
	virtual ~mthread(){}
public:
	virtual bool initinstance();
	virtual void mainfunction();
	virtual unsigned int exitinstance();
public:
	bool start();
	bool isrun()const{return brun;} 
	void stop(){brun = false;}
	void setrun(bool bl){brun = bl;}
	unsigned long threadid()const{return tid;}
   
protected:
    void millisleep( unsigned int uimillisecond);
private:

#ifdef _WIN32
    static DWORD WINAPI threadfunction(void* param);
#else
    static void* threadfunction(void* param);
#endif

	unsigned int run();
private:
	bool brun;
	unsigned long tid;
};


class falselockx_
{
public:
	void Lock() {}
	void Unlock(){}
};
class mlockx_
{
private:
#ifdef _WIN32
	CRITICAL_SECTION m_syslock;
#else
	pthread_mutex_t m_syslock;
#endif

public:
	mlockx_() 
	{
#ifdef _WIN32
	::InitializeCriticalSection(&m_syslock);
#else
	pthread_mutex_init(&m_syslock,NULL);
#endif
	}
	~mlockx_() 
	{
#ifdef _WIN32
		::DeleteCriticalSection(&m_syslock);
#else
		pthread_mutex_destroy(&m_syslock);
#endif
	}

public:
	void Lock() 
	{
#ifdef _WIN32
	::EnterCriticalSection(&m_syslock);
#else
	pthread_mutex_lock(&m_syslock);
#endif
	}

	void Unlock() 
	{
#ifdef _WIN32
	::LeaveCriticalSection(&m_syslock);
#else
	pthread_mutex_unlock(&m_syslock);
#endif
	}

	void* operator()()
	{
		return (void*)&m_syslock ;
	}
};

template<typename l_>
class lockx_
{
public:
	lockx_(l_* plockobject)
	{
		m_plock = plockobject;
		m_plock->Lock();
	}
	~lockx_()
	{
		m_plock->Unlock();
	}
private:
	lockx_(){};
	l_* m_plock;
};


#endif	//THREAD_MULTI_PLAT_H_