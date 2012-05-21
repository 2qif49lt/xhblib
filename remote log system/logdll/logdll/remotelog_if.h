#ifndef REMOTELOG_INTERFACE_XHB_H_
#define REMOTELOG_INTERFACE_XHB_H_

#ifdef REMOTELOG_DLL_IMPLEMENT_
#define rtlog_dll _declspec(dllexport)
#else
#define rtlog_dll _declspec(dllimport)
#endif

/*
iloglv:
"FATE   ",    //0 
"2012   ",    //1
"FAIL   ",    //2
"ERROR  ",    //3
"WARN   ",    //4
"INFO   ",    //5
"DEBUG  ",    //6
"SECRET ",    //7
*/

#define MAX_LOG_LEN 1024

int rtlog_dll init_rlog(unsigned int uisrvid,const char* szlocalip,const char* szsrvip,unsigned short ussrvport,int iloglv);

int rtlog_dll send_rlog(int iloglv,const char* szlog);

void rtlog_dll stop_rlog();

#endif //REMOTELOG_INTERFACE_XHB_H_