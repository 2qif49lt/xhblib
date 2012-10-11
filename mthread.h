#ifndef THREAD_MULTI_PLAT_H_
#define THREAD_MULTI_PLAT_H_

//windows
#include "Log_/lockx.h"
using namespace xhb;
#ifdef _WIN32
//#pragma comment(lib,"pthreadVC2.lib")   //windows need pthread dll
//#include "winsock2.h"
#include "windows.h"
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include <stdio.h>


class mthread
{
public:
	mthread():brun(true),tid(0),hdle(NULL){}
	virtual ~mthread();
public:
	virtual bool initinstance();
	virtual void mainfunction();
	virtual unsigned int exitinstance();
public:
	bool start();
	bool isrun()const{return brun;} 
	virtual void stop(){brun = false;}
	void setrun(bool bl){brun = bl;}
	unsigned long threadid()const{return tid;}
	void waitend();
protected:
	//millisleep 线程sleep. uimircosecond 单位是毫秒
	void millisleep( unsigned int uimillisecond);
private:

	//threadfunction 线程主函数

#ifdef _WIN32
	static DWORD WINAPI threadfunction(void* param);
#else
	static void* threadfunction(void* param);
#endif

	//run 框架
	unsigned int run();
private:
	//brun 线程运行状态


	bool brun;

	//threadid 线程id
	unsigned long tid;
	void* hdle;	//线程句柄
};



#endif	//THREAD_MULTI_PLAT_H_