#ifndef LOGMGR_LOGSRV_XHB_H_
#define LOGMGR_LOGSRV_XHB_H_
#include "logx.h"
#include <stdio.h>
#include <time.h>
#include <locale.h>


#define  g_plogfolder   "logdb" 
template<
	long ulmaxfilesize = 10 * 1024 * 1024,
	int nmaxlogfilenum = 10,
	loghandlefn loghandT = loghandledefault>
	class logmgr
	{
	public:
		logmgr()
		{
			m_file = NULL;
			memset(m_folder,0,sizeof(m_folder));
			memset(m_tmpbuf,0,sizeof(m_tmpbuf));
		}
		~logmgr()
		{
			closestream();
			char* pfilename = NULL;
			while (pfilename = m_filequeue.pop_front())
			{
				delete [] pfilename;
			}
		}
		bool init(unsigned int uisid,const char* pinternetip,const char* plocalip)
		{
			if(access(g_plogfolder,0) == -1)
				mkdir(g_plogfolder,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			sprintf(m_folder,"%s/%u/",g_plogfolder,uisid);
			if(access(m_folder,0) == -1)
				mkdir(m_folder,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			sprintf(m_folder,"%s/%u/%s",g_plogfolder,uisid,pinternetip);
			if(access(m_folder,0) == -1)
				mkdir(m_folder,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			sprintf(m_folder,"%s/%u/%s/%s",g_plogfolder,uisid,pinternetip,plocalip);
			if(access(m_folder,0) == -1)
				mkdir(m_folder,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            time_t curtime = time(NULL);
            tm tim = *localtime(&curtime); 
            char chlastfolder[64];
            sprintf(chlastfolder,"%04d%02d%02d%02d%02d%02d",tim.tm_year + 1900,
                tim.tm_mon + 1,tim.tm_mday,tim.tm_hour,tim.tm_min,tim.tm_sec);
            sprintf(m_folder,"%s/%u/%s/%s/%s",g_plogfolder,uisid,pinternetip,plocalip,chlastfolder);
            if(access(m_folder,0) == -1)
                mkdir(m_folder,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			return createstream();
		}
		bool log(unsigned int uitimet,short slv,unsigned int tid,char* plog)
		{
			if (m_file == NULL)
				return false;
			else
			{
				if (ftell(m_file) >= ulmaxfilesize)
				{
					if(!createstream())
						return false;
				}
				
				time_t curtime = uitimet;
				tm tim = *localtime(&curtime); 
				sprintf(m_tmpbuf,"[ %08u %04d-%02d-%02d %02d:%02d:%02d <%s> ]  [",
					tid,
					tim.tm_year + 1900,
					tim.tm_mon + 1,
					tim.tm_mday,
					tim.tm_hour,
					tim.tm_min,
					tim.tm_sec, 
					g_getloglvtxt(slv));
				fputs(m_tmpbuf,m_file);
				fputs(plog,m_file);
				fputs("]\n",m_file);
			}
			return true;		
		}
		void closelog()
		{
			closestream();
			char* pfilename = NULL;
			while (pfilename = m_filequeue.pop_front())
			{
				delete [] pfilename;
			}
		}
	private:
		bool createstream()
		{
			closestream();

			char sznewname[260] = {0};

			strcpy(sznewname,m_folder);
			strcat(sznewname,createfilename());

			m_file = fopen(sznewname,"w+");
			if (NULL == m_file)
				return false;
 
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
		void closestream()
		{
			if (m_file)
			{
				fclose(m_file);
				m_file = NULL;
			}
		}
		inline char* createfilename()
		{
			time_t curtime = time(NULL);
			tm tim = *localtime(&curtime); 
			sprintf(m_tmpbuf,"/%04d%02d%02d%02d%02d%02d.log",tim.tm_year + 1900,
				tim.tm_mon + 1,tim.tm_mday,tim.tm_hour,tim.tm_min,tim.tm_sec);
			return m_tmpbuf;
		}
		FILE* m_file;
		char m_folder[260];
		char m_tmpbuf[260];
		ptrqueuex<char,nmaxlogfilenum,true>	m_filequeue;
		
	};
#endif //LOGMGR_LOGSRV_XHB_H_