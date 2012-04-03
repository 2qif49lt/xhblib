#ifndef IOCPX_NET_XHB_H_
#define IOCPX_NET_XHB_H_

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")
#include "winsock2.h"
#include "Mswsock.h"

#include "msgheadx.h"
#include "memoryx.h"
#include "ptrqueuex.h"
#include "lockx.h"
#include "linkx.h"
#include "listenthreadx.h"
#include "workerthreadx.h"
#include "iocpinterfacex.h"
#include <map>
using std::map;

namespace xhb
{
	template<typename m_>
	class tcplinkx_;

	template<typename m_ = mm_char_defaultx_>
	class iocpx_
	{
	public:
		iocpx_()
		{
			m_handleinterface			= NULL;
			m_hCompletionPort			= INVALID_HANDLE_VALUE;
			m_WorkerThreadNum			= 0;
			m_pworkthreadarr			= NULL;
			m_plistenthread				= NULL;
			m_uisendbufflen				= 0;
			m_uirecvbufflen				= 0;
			m_maxlink					= 0;
			m_brunning					= false;
		}
		~iocpx_()
		{
			if (m_plistenthread != NULL)
				delete m_plistenthread;
			if (m_pworkthreadarr != NULL)
				delete [] m_pworkthreadarr;
			if (m_hCompletionPort != NULL)
				CloseHandle(m_hCompletionPort);
		}
	public:
		bool start_iocp(DWORD			dwlistenip,//监听的服务器的IP(0表示监听本地所有ip)
			unsigned short				uslistenport,//监听的Port(0:表示不用起监听端口)
			unsigned int				uimaxconnectnum,//最大的连接数
			unsigned int				uirecvbufflen,//接受缓冲存储的数据的最大长度
			unsigned int				uisendbufflen,//发送缓冲存储的数据的最大长度
			iocpinterfacex_*			piocpinterface)//接收IOCP收到的数据的回调指针(用户实现该函数)
		{
			
			m_maxlink = uimaxconnectnum;
			m_uirecvbufflen = uirecvbufflen;
			m_uisendbufflen = uisendbufflen;

			WSADATA wsaData;
			if (WSAStartup(0x0202, &wsaData) != 0)
				return false;
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			SYSTEM_INFO SysInfo;

			if (piocpinterface == NULL)
				return false;
			m_handleinterface = piocpinterface;
			m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
			if (0 == m_hCompletionPort)
				return false;
			GetSystemInfo(&SysInfo);
			m_WorkerThreadNum  = SysInfo.dwNumberOfProcessors * 2;
			
			m_pworkthreadarr = new(nothrow) workerthreadx<m_>[m_WorkerThreadNum];
			if (m_pworkthreadarr == NULL)
				return false;
			m_brunning = true;
			for (int i = 0 ; i != m_WorkerThreadNum; ++i)
			{
				m_pworkthreadarr[i].m_piocpx = this;
				m_pworkthreadarr[i].Start();
			}
			if (uslistenport != 0)
			{
				m_plistenthread = new(nothrow) listenthreadx<m_>;
				if(m_plistenthread == NULL)
					return false;
				m_plistenthread->m_piocpx = this;
				m_plistenthread->m_sPort = uslistenport;
				m_plistenthread->Start();
			}
			
			return true;
		}
		bool send(unsigned long ulsocketid,char* chbuff,unsigned int uilen)
		{
			tcplinkx_<m_>* plink = findlink(ulsocketid);
			if (plink == NULL)
				return false;
			else
			{
				if (uilen > m_uisendbufflen)
					return false;
				else
					return plink->post_send(chbuff,uilen);
			}
		}
		//停止NetIOCP服务器
		void stop_iocp()
		{
			m_brunning = false;
			Sleep(2000);
			for (int i = 0; i != m_WorkerThreadNum; ++i)
			{
				int trywait = 0;
				while(m_pworkthreadarr[i]->IsRunning() && trywait < 3)
				{
					Sleep(1000);
					++trywait;
				}
				if (trywait == 3)
					m_pworkthreadarr[i]->Terminate();
			}
			int trywait = 0;
			while(m_plistenthread->IsRunning() && trywait < 3)
			{
				Sleep(1000);
				++trywait;
			}
			if (trywait == 3)
				m_plistenthread->Terminate();
			CloseHandle(m_hCompletionPort);
			m_hCompletionPort = NULL;
			clearlink();
		};

		void close_link(tcplinkx_<m_>* plink)
		{
			if (plink->m_bdelete == true)
			{
				handle_closed(plink);
				eraselink(plink->m_dwSocketID);
				m_::free(plink);
				return;
			}
			else
			{
				plink->m_bdelete = true;
				plink->shut_down();
				plink->close_socket();
				if (plink->m_brecv == false && plink->m_bsend == false)
				{
					handle_closed(plink);
					eraselink(plink->m_dwSocketID);
					m_::free(plink);
				}
			}
		}
		bool handle_recv_msg(tcplinkx_<m_>* plink,char* pmsg,unsigned int uimsglen)
		{
			if(uimsglen< sizeof(BaseMsgHead))
				return false;
			return m_handleinterface->on_recv_msg(pmsg,uimsglen,plink->m_dwpeerip,plink->m_usport,plink->m_dwSocketID) == 0;
		}
		bool handle_accept_link(tcplinkx_<m_>* plink)
		{
			return m_handleinterface->on_accepted(plink->m_dwpeerip,plink->m_usport,plink->m_dwSocketID) == 0;
		}
		void handle_closed(tcplinkx_<m_>* plink)
		{
			m_handleinterface->on_closed(plink->m_dwpeerip,plink->m_usport,plink->m_dwSocketID);
		}
		void handle_complate_notify(DWORD dwtrans,tcplinkx_<m_>* plink,pper_io_objx_ pperobject)
		{
			switch(pperobject->operation)
			{
			case OP_ACCEPT:		//这是从Listen线程过来的消息
				{
					plink->m_recvobjx.operation = OP_RECV;
					plink->m_sendobjx.operation = OP_SEND;
					if (addlink(plink))
					{	
						if (handle_accept_link(plink))
						{
							if(plink->post_recv() == false)
								close_link(plink);
						}
						else
							close_link(plink);
					}
					else
						close_link(plink);
				}
				break;
			case OP_RECV:
				{				
					if(dwtrans == 0)
					{
						//处理异常信息
						plink->m_brecv = false;
						close_link(plink);		
					}
					else
					{
						//处理该接受的数据	
						if(plink->onrecv(dwtrans))
							plink->post_recv();	
					} 
				}
				break;
			case OP_SEND:
				{
					if(dwtrans == 0)	
					{
						//处理异常信息
						plink->m_bsend = false;				
						close_link(plink);									
					}
					else
					{
						plink->onsend(dwtrans);	
					} 
				}
				break;
			default:
				break;
			}
		}

	private:
		inline tcplinkx_<m_>* findlink(unsigned long ulsocketid)
		{
			_lockx lock(&m_lock);
			linkmapx_::iterator it = m_linkmap.find(ulsocketid);
			if (it == m_linkmap.end())
				return NULL;
			else
				return it->second;
		}
		inline bool addlink(tcplinkx_<m_>* plink)
		{
			_lockx lock(&m_lock);
			linkmapx_::iterator it = m_linkmap.find(plink->m_dwSocketID);
			if (it == m_linkmap.end())
			{
				m_linkmap[plink->m_dwSocketID] = plink;
				return true;
			}
			return false;
		}
		inline void eraselink(unsigned long ulsocketid)
		{
			_lockx lock(&m_lock);
			linkmapx_::iterator it = m_linkmap.find(ulsocketid);
			m_linkmap.erase(ulsocketid);
		}
		void clearlink()
		{
			for (linkmapx_::iterator it = m_linkmap.begin(); it != m_linkmap.end(); ++it)
			{
				tcplinkx_<m_>* plink = it->second;
				if (plink != NULL)
				{
					plink->shut_down();
					plink->close_socket();
					m_::free(plink);
				}
			}
			m_linkmap.clear();
		}
	public: 
		bool							m_brunning;
		HANDLE							m_hCompletionPort;	//完成端口句柄
		unsigned int					m_uisendbufflen;
		unsigned int					m_uirecvbufflen;
		unsigned int					m_maxlink;
	private:
		iocpinterfacex_*				m_handleinterface;	//外界提供的接口
		
		DWORD							m_WorkerThreadNum;	//工作器线程个数
		workerthreadx<m_>*				m_pworkthreadarr;		//工作器线程数组
		listenthreadx<m_>*				m_plistenthread;	//监听线程
		
		typedef map<unsigned long,tcplinkx_<m_>* > linkmapx_;
		
		linkmapx_						m_linkmap;
		criticalsectionx_				m_lock;
	};

};
#endif	//IOCPX_NET_XHB_H_