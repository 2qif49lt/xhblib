#ifndef LOGXL_SYSTEM_XHB_H_
#define LOGXL_SYSTEM_XHB_H_

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include <new>
#include "ptrqueuex.h"
#include "mthread.h"
using namespace std;
using namespace xhb;
#define LOG_LV_2012   1
#define LOG_LV_FAILD   2
#define LOG_LV_ERROR   3
#define LOG_LV_WARN   4
#define LOG_LV_INFO   5 
#define LOG_LV_NORMAL LOG_LV_INFO  
#define LOG_LV_DEBUG   6
#define LOG_LV_PRIVATE   7
#define LOG_ALIGN	"\n\t\t\t\t    "
	inline char* g_getloglvtxt(unsigned int nlv)
	{
		static char* g_loglvtxt[] = {
    "FATE   ",    //0 
    "2012   ",    //1
    "FAIL   ",    //2
    "ERROR  ",    //3
    "WARN   ",    //4
    "INFO   ",    //5
    "DEBUG  ",    //6
    "SECRET ",    //7
		};
		return g_loglvtxt[nlv % 8];
	}

	typedef void (*loghandlefn)(char*);


	inline void loghandledefault(char* pfilename)
	{
		remove(pfilename);
	}

	template<typename l_ =  falselockx_,
		long ulmaxfilesize = 10 * 1024 * 1024,
		int nmaxlogfilenum = 10,
		int nflush = 1,
		loghandlefn loghandT = loghandledefault>
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
			memset(m_szfilename,0,sizeof(m_szfilename) * sizeof(char));
            memset(m_szfolderpath,0,sizeof(m_szfolderpath) * sizeof(char));
 		}
 		virtual ~logx()
 		{
 			CloseStream();
 			if (m_logbuff != NULL)
 			{
 				delete [] m_logbuff; 
 				m_logbuff = NULL;
 			}
			char* pfilename = NULL;
			while (pfilename = m_filequeue.pop_front())
			{
				delete [] pfilename;
			}
 		}
 	public:
 		bool InitLog(const char* pstrfolderpath = NULL,const char* pstrfilename = NULL,unsigned int nwritelevel = LOG_LV_NORMAL)
 		{
 			if (m_nwritelevel != 0)
 			{
 				return false;
 			}
	//		setlocale(LC_ALL,"");
 		
 			m_nwritelevel = nwritelevel;
            
            if(m_logbuff == NULL)
            {
 			    m_logbuff = new(nothrow) char[4096];
 			    if(m_logbuff ==NULL)
 				    return false;
            }
    
 			if (pstrfilename!= NULL && pstrfolderpath  != NULL)
 			{		
 				strcpy(m_szfilename,pstrfilename);
 			    strcpy(m_szfolderpath,pstrfolderpath);

                if(access(m_szfolderpath,0) == -1)
                    mkdir(m_szfolderpath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                char chEnds = m_szfolderpath[strlen(m_szfolderpath) - 1];
                if (chEnds != '\\' || chEnds != '/')
                {
                    m_szfolderpath[strlen(m_szfolderpath)] = '/';;
                    m_szfolderpath[strlen(m_szfolderpath) + 1] = 0;
                }
                

                m_bstdout = false;
 				if (!CreateStream())
 					return false;
 			}
			else
				m_stream = stdout;
 			return true;
 		}

 		bool log(const char* strlog,unsigned int nwritelevel = LOG_LV_INFO)
 		{
 			lockx_<l_> lock(&m_lock);
          
 			if (nwritelevel <= m_nwritelevel)
 			{
 				if (m_bstdout)
				{
					GetLogTimeStr(nwritelevel);

					fputs(m_timebuff,m_stream);
					fputs(strlog,m_stream);
                    fputs("\n",m_stream);
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
					fputs(m_timebuff,m_stream);
 					fputs(strlog,m_stream);
					fputs("\n",m_stream);

 					if(nflush == 1)	
 						fflush(m_stream);
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
 		bool log(unsigned int nwritelevel,char*  pszFormat,...)
 		{
 			lockx_<l_> lock(&m_lock);

 			if (nwritelevel <= m_nwritelevel)
 			{
 				va_list args;
 				va_start(args, pszFormat);
 				vsnprintf(m_logbuff,4096,pszFormat, args);
 				va_end(args);
 
 				if (m_bstdout)
				{
					GetLogTimeStr(nwritelevel);
					fputs(m_timebuff,m_stream);
					fputs(m_logbuff,m_stream);
                    fputs("\n",m_stream);
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
					fputs(m_timebuff,m_stream);
 					fputs(m_logbuff,m_stream);
                    fputs("\n",m_stream);
 
 					if(nflush == 1)	
 						fflush(m_stream);
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
 			sprintf(m_timebuff,"_%04d%02d%02d%02d%02d%02d",tim.tm_year + 1900,
				tim.tm_mon + 1,tim.tm_mday,tim.tm_hour,tim.tm_min,tim.tm_sec);
 		}
		inline void GetLogTimeStr(unsigned int nlv)
		{
			time_t curtime = time(NULL);
			tm tim = *localtime(&curtime); 
			sprintf(m_timebuff,"%06d %04d-%02d-%02d %02d:%02d:%02d [%s]   ",
                (int)syscall(SYS_gettid),
                tim.tm_year + 1900,
				tim.tm_mon + 1,
                tim.tm_mday,
                tim.tm_hour,
                tim.tm_min,
                tim.tm_sec, 
                g_getloglvtxt(nlv));
		}
 		void CloseStream()
 		{
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

			char sznewname[260] = {0};
            
            strcpy(sznewname,m_szfolderpath);

			char* plastfindptr = strrchr(m_szfilename,'.');
			if (plastfindptr == NULL)
			{
				return false;
			}
			int nleftcharnum = plastfindptr - m_szfilename;
			strncat(sznewname,m_szfilename,nleftcharnum);
			GetFileTimeStr();
			strcat(sznewname,m_timebuff);
			strcat(sznewname,plastfindptr);

			m_stream = fopen(sznewname,"w+");
 			if (NULL == m_stream)
 			{
 				return false;
 			}
			if (m_filequeue.isfull())
			{
				char* pfilename = m_filequeue.pop_front();
				if (pfilename)
				{
					loghandT(pfilename);
					strcpy(pfilename,sznewname);
					m_filequeue.push_back(pfilename);
				}
			}
			else
			{
				char* pfilename = new(nothrow) char[260];
				if(pfilename == NULL)
					return false;
				strcpy(pfilename,sznewname);
				m_filequeue.push_back(pfilename);
			}	 
 			return true;
 		}
 	private:
 		int m_nwritecount;
 		char* m_logbuff;
 		FILE* m_stream;
 		unsigned int m_nwritelevel;
 		l_ m_lock;
		char m_timebuff[128];
        bool m_bstdout;
		char m_szfilename[260];
		char m_szfolderpath[260];
		ptrqueuex<char,nmaxlogfilenum,true>	m_filequeue;
 	};

#endif //LLOGX_SYSTEM_XHB_H_

