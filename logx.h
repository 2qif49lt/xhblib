#ifndef LOGX_SYSTEM_XHB_H_
#define LOGX_SYSTEM_XHB_H_
/************************************************************************/
/* 
日志类(vs2008无错,05以下可能会少头文件和警告)
支持unicode 多线程安全,可以自定义线程锁,自动建立新文件,根据调试等级下入日志,调节写入速度
运行时更改写入等级。可变长打印，变长最大支持4K长度。
可以标准输出stdout，也可以写入文件
可配最多保留日志个数，以防止程序出错,日志文件塞满硬盘
可配处理日志接口,你可以将日志发送出去再删除等等,默认是直接删除
内部c实现,高效.
使用固定内存(保存最近N个文件路径最终是趋于固定内存)。
使用方法
basefunctionx_::initiate();
string strpath = basefunctionx_::get_program_path();
strpath += "\\log";
logx<> log;
log.InitLog(NULL,NULL,LOG_LV_PRIVATE);	
int i = 0;
while (++i)
{
log.log(i%LOG_LV_PRIVATE + 1,_T("第 %08d"),i);
}
打印样式:
2011-05-27 23:22:20  [信息]  xxxx
2011-05-27 23:22:20  [调试]  yyyy
2011-05-27 23:22:20  [错误]  zzzz

使用多线程打印时 需要提供
class logfreelockx
{
public:
void Lock() {}
void Unlock(){}
};
的接口类型

//处理日志文件的接口,参数为日志类提供的文件路径+名字
typedef void (*loghandlefn)(TCHAR*);

默认参数为最优,但也可以自己设置以适用自己的情况

待增加功能: 写远程日志(日志服务器)
*/
/************************************************************************/


#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <direct.h>

#include "ptrqueuex.h"		//使用简单指针队列
#include "lockx.h"

#include "windows.h"

#pragma warning(push)
#pragma warning(disable:4996)	//去除安全函数警告
#pragma warning(disable:4244)
namespace xhb
{
#define LOG_LV_2012   1     //惨剧人寰 世界末日
#define LOG_LV_FAILD   2        //失败
#define LOG_LV_ERROR   3        //错误
#define LOG_LV_WARN   4        //警告
#define LOG_LV_INFO   5        //信息
#define LOG_LV_NORMAL LOG_LV_INFO    //正常
#define LOG_LV_DEBUG   6        //调试
#define LOG_LV_PRIVATE   7       //私密
#define LOG_ALIGN	"\n\t\t\t\t    "		//方便log换行对齐     
	inline char* g_getloglvtxt(unsigned int nlv)
	{
		static TCHAR* g_loglvtxt[] = {
			_T("惨剧人寰"),    //0 占位
			_T("世界末日"),    //1
			_T("失败"),    //2
			_T("错误"),    //3
			_T("警告"),    //4
			_T("信息"),    //5
			_T("调试"),    //6
			_T("私密"),    //7
		};
		return g_loglvtxt[nlv];
	}
	//处理日志文件的接口,参数为日志类提供的文件路径+名字
	typedef void (*loghandlefn)(TCHAR*);

	//默认日志处理函数,删除日志文件
	//你可以定义相同类型的函数,里面做其他操作
	inline void loghandledefault(TCHAR* pfilename)
	{
		_tremove(pfilename);
	}

	template<typename l_ =  falselockx_,		//安全锁

		long ulmaxfilesize = 10 * 1024 * 1024,		//单个日志文件大小

		int nmaxlogfilenum = 10,					//最多保存日志文件个数,至少是大于等于1的数													

		int nflush = 1,								//表示写日志多少次刷新一次文件缓存,
													//如果值为0则表示系统自己刷新
													//大于等于1表示多少次刷新一次

		loghandlefn loghandT = loghandledefault		//日志处理函数,默认删除
	>
 	class logx
 	{
 	public:
 		logx()
 		{
 			m_nwritelevel = 0;
 			m_nwritecount = 0;
 			m_logbuff = NULL;
			m_stream = NULL;
            m_bstdout = true;
			memset(m_szfilename,0,sizeof(m_szfilename) * sizeof(TCHAR));
            memset(m_szfolderpath,0,sizeof(m_szfolderpath) * sizeof(TCHAR));
 		}
 		virtual ~logx()
 		{
 			CloseStream();
 			if (m_logbuff != NULL)
 			{
 				delete [] m_logbuff; 
 				m_logbuff = NULL;
 			}
			TCHAR* pfilename = NULL;
			while (pfilename = m_filequeue.pop_front())
			{
				delete [] pfilename;
			}
 		}
 	public:
		/*
		初始化,必须首先调用；
        pstrfolderpath: 日志目录全路径,可以为NULL.path必须存在或其上层目录必须已经建立
		pstrfilename:文件名,必须有.后缀名,可以为NULL，路径或文件名为空时表示用标准输出
		nwritelevel 打印or写入等级
		*/
 		bool InitLog(const TCHAR* pstrfolderpath = NULL,const TCHAR* pstrfilename = NULL,unsigned int nwritelevel = LOG_LV_NORMAL)
 		{
 			if (m_nwritelevel != 0)
 			{
 				return false;	//防止重复调用
 			}
			setlocale(LC_ALL,"");
 		
 			m_nwritelevel = nwritelevel;
 
 			m_logbuff = new(nothrow) TCHAR[4096];
 			if(m_logbuff ==NULL)
 				return false;
    
 			if (pstrfilename!= NULL && pstrfolderpath  != NULL)
 			{		
 				_tcscpy(m_szfilename,pstrfilename);
 			    _tcscpy(m_szfolderpath,pstrfolderpath);

                TCHAR chEnds = m_szfolderpath[_tcslen(m_szfolderpath) - 1];
                if (chEnds != _T('\\') || chEnds != _T('/'))
                {
                    m_szfolderpath[_tcslen(m_szfolderpath)] = _T('\\');;
                    m_szfolderpath[_tcslen(m_szfolderpath) + 1] = 0;
                }
                if(_taccess(m_szfolderpath,0) == -1)
                    _tmkdir(m_szfolderpath);

                m_bstdout = false;
 				if (!CreateStream())
 					return false;
 			}
			else
				m_stream = stdout;
 			return true;
 		}
 		bool log(const TCHAR* strlog,unsigned int nwritelevel = LOG_LV_INFO)
 		{
 			lockx_<l_> lock(&m_lock);
          
 			if (nwritelevel <= m_nwritelevel)
 			{
 				if (m_bstdout)
				{
					GetLogTimeStr(nwritelevel);

					_fputts(m_timebuff,m_stream);
					_fputts(strlog,m_stream);
                    _fputts("\n",m_stream);
				}
 				else
 				{
                    if(m_stream == NULL)
                        return false;
 					if (ftell(m_stream) >= ulmaxfilesize)
 					{
 						if (!CreateStream())
 							return false;
 					}
					GetLogTimeStr(nwritelevel);
					_fputts(m_timebuff,m_stream);
 					_fputts(strlog,m_stream);
					_fputts("\n",m_stream);

 					//如果1则一直刷新
 					if(nflush == 1)	
 						fflush(m_stream);
 					//如果是大于1的数则定次刷新
 					else if (nflush > 1)	
 					{
 						++m_nwritecount;
 						if (m_nwritecount >= nflush)
 						{
 							fflush(m_stream);
 							m_nwritecount = 0;
 						}
 					}
 				}
 			}
 			return true;
 		}
 		bool log(unsigned int nwritelevel,TCHAR*  pszFormat,...)
 		{
 			lockx_<l_> lock(&m_lock);

 			if (nwritelevel <= m_nwritelevel)
 			{
 				va_list args;
 				va_start(args, pszFormat);
 				_vsntprintf(m_logbuff,4096,pszFormat, args);
 				va_end(args);
 
 				if (m_bstdout)
				{
					GetLogTimeStr(nwritelevel);
					_fputts(m_timebuff,m_stream);
					_fputts(m_logbuff,m_stream);
                    _fputts("\n",m_stream);
				}
 				else
 				{
                    if(m_stream == NULL)
                        return false;

 					if (ftell(m_stream) >= ulmaxfilesize)
 					{
 						if (!CreateStream())
 							return false;
 					}
					GetLogTimeStr(nwritelevel);
					_fputts(m_timebuff,m_stream);
 					_fputts(m_logbuff,m_stream);
                    _fputts("\n",m_stream);
 
 					//如果1则一直刷新
 					if(nflush == 1)	
 						fflush(m_stream);
 					//如果是大于1的数则定次刷新
 					else if (nflush > 1)	
 					{
 						++m_nwritecount;
 						if (m_nwritecount >= nflush)
 						{
 							fflush(m_stream);
 							m_nwritecount = 0;
 						}
 					}
 				}
 			}
 			return true;
 		}
 		unsigned int  SetWriteLevel(unsigned int nlevel)
 		{
            unsigned int uiret = m_nwritelevel;
 			m_nwritelevel = nlevel;
            return uiret;
 		}
 	private:
 		inline void GetFileTimeStr()
 		{
 			time_t curtime = time(NULL);
 			tm tim = *localtime(&curtime); 
 			_stprintf(m_timebuff,_T("_%04d%02d%02d%02d%02d%02d"),tim.tm_year + 1900,
				tim.tm_mon + 1,tim.tm_mday,tim.tm_hour,tim.tm_min,tim.tm_sec);
 		}
		inline void GetLogTimeStr(unsigned int nlv)
		{
            //static TCHAR tmpbuff[128];
			time_t curtime = time(NULL);
			tm tim = *localtime(&curtime); 
			_stprintf(m_timebuff,_T("%06d %04d-%02d-%02d %02d:%02d:%02d [%s]   "),
                GetCurrentThreadId(),
                tim.tm_year + 1900,
				tim.tm_mon + 1,
                tim.tm_mday,
                tim.tm_hour,
                tim.tm_min,
                tim.tm_sec, 
                g_getloglvtxt(nlv));
            //换种风格显示，可以去重.
//             if (_tcscmp(m_timebuff,tmpbuff) == 0)
//             {
//                 _stprintf(m_timebuff,_T("%36s"),_T(""));
//             }
//             else
//                 memcpy(tmpbuff,m_timebuff,sizeof(tmpbuff) * sizeof(TCHAR));
		}
 		void CloseStream()
 		{
			//简单判断终端是否是文件
 			if (m_bstdout)
 				return;
 			if (m_stream)
 			{
 				fclose(m_stream);
				m_stream = NULL;
 			}
 		}
 		bool CreateStream()
 		{
 			CloseStream();

			TCHAR sznewname[260] = {0};
            
           _tcscpy(sznewname,m_szfolderpath);

			TCHAR* plastfindptr = _tcsrchr(m_szfilename,_T('.'));
			if (plastfindptr == NULL)
			{
				return false;
			}
			int nleftcharnum = plastfindptr - m_szfilename;
			_tcsncat(sznewname,m_szfilename,nleftcharnum);
			GetFileTimeStr();
			_tcscat(sznewname,m_timebuff);
			_tcscat(sznewname,plastfindptr);

			m_stream = _tfopen(sznewname,_T("w+"));
 			if (NULL == m_stream)
 			{
 				return false;
 			}
			if (m_filequeue.isfull())
			{
				TCHAR* pfilename = m_filequeue.pop_front();
				if (pfilename)
				{
					//处理文件
					loghandT(pfilename);
					_tcscpy(pfilename,sznewname);
					m_filequeue.push_back(pfilename);
				}
			}
			else
			{
				TCHAR* pfilename = new(nothrow) TCHAR[260];
				if(pfilename == NULL)
					return false;
				_tcscpy(pfilename,sznewname);
				m_filequeue.push_back(pfilename);
			}	 
 			return true;
 		}
 	private:

 		int m_nwritecount;
 		TCHAR* m_logbuff;

 		FILE* m_stream;
 		unsigned int m_nwritelevel;			//写日志等级

 		l_ m_lock;				//同步锁
		
		TCHAR m_timebuff[128];		//用于取时间的缓冲区

        bool m_bstdout;
		TCHAR m_szfilename[260];			//文件全路径名
		TCHAR m_szfolderpath[260];          //日志文件目录

		ptrqueuex<TCHAR,nmaxlogfilenum,true>	m_filequeue;		
 	};
}
#pragma warning(pop)
#endif //LOGX_SYSTEM_XHB_H_
