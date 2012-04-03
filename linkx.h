#ifndef LINKX_NET_XHB_H_
#define LINKX_NET_XHB_H_
#include "ptrqueuex.h"
#include "memoryx.h"
#include "msgheadx.h"
#include "iocpx.h"
namespace xhb
{
	template<typename m_>
	class iocpx_;
	typedef struct tagsendstructx_
	{
		char*					pbuff;
		unsigned int			uilen; 
	}sendstructx_,*psendstructx_;
	typedef struct _per_io_obj
	{
		WSAOVERLAPPED       ol;
		WSABUF				wbuf;
		int					operation;     // Type of operation issued
		
#define OP_ACCEPT       0                 // AcceptEx
#define OP_RECV         1                 // WSARecv/WSARecvFrom
#define OP_SEND        2                 // WSASend/WSASendTo
	} per_io_objx_,*pper_io_objx_;

	template<typename m_ = mm_char_defaultx_>
	class tcplinkx_
	{
	public:
		tcplinkx_()
		{
			m_sendobjx.operation = OP_SEND;
			m_recvobjx.operation = OP_RECV;
			m_chsendbuff = NULL;
			m_chrecvbuff = NULL;
			m_dwSocketID = 0;
			m_ssocket = INVALID_SOCKET;
			m_bsend = false;
			m_brecv = false;
			m_uirecved = 0;
			m_uineedsend = 0;
			m_uisended = 0;
			m_dwlastalivetime = GetTickCount();
			m_bdelete = false;
		}
		~tcplinkx_()
		{
			if (m_chrecvbuff != NULL)
				m_::free(m_chrecvbuff);
			clear_link();
		}
		inline bool init_buff(unsigned int uibufflen)
		{ 
			m_chrecvbuff = (char*)m_::alloc(uibufflen); 
			if (m_chrecvbuff == NULL)
				return false;
			m_uirecvbufflen = uibufflen;
			m_uirecved = 0;
			return true;
		}

		bool overlapped_first_send()
		{
			ZeroMemory(&(m_sendobjx.ol), sizeof(WSAOVERLAPPED));
			m_sendobjx.wbuf.buf = m_chsendbuff;
			m_sendobjx.wbuf.len = min(m_uineedsend,g_uipersendsize);
			int rc = WSASend(
				m_ssocket,
				&m_sendobjx.wbuf,
				1,
				NULL,
				0,
				&m_sendobjx.ol,
				NULL
				);

			if (rc == SOCKET_ERROR)
			{
				int err;
				if ((err = WSAGetLastError()) != WSA_IO_PENDING)
				{
					if (err == WSAENOBUFS)
						cout<<"overlapped_first_send: WSASend* failed! "<<WSAGetLastError()<<endl; 
					return false;
				}
			}
			return true;
		}
		bool overlapped_next_send()
		{
			ZeroMemory(&(m_sendobjx.ol), sizeof(WSAOVERLAPPED));
			m_sendobjx.wbuf.buf = m_chsendbuff + m_uisended;
			m_sendobjx.wbuf.len = min(m_uineedsend - m_uisended,g_uipersendsize);
			int rc = WSASend(
				m_ssocket,
				&m_sendobjx.wbuf,
				1,
				NULL,
				0,
				&m_sendobjx.ol,
				NULL
				);

			if (rc == SOCKET_ERROR)
			{
				int err;
				if ((err = WSAGetLastError()) != WSA_IO_PENDING)
				{
					if (err == WSAENOBUFS)
						cout<<"overlapped_next_send: WSASend* failed! "<<WSAGetLastError()<<endl; 
					return false;
				}
			}
			return true;
		}
		bool post_send(char* chdata,unsigned int ndatalen)
		{
			_lockx lock(&m_lockobject);
			
			if (m_sendqueuex.isfull())
				return false;
			else
			{
				char* pchtmp = m_::alloc(sizeof(sendstructx_));
				if (pchtmp == NULL)
					return false;
				char* ptmpbuff = m_::alloc(sizeof(ndatalen));
				if (ptmpbuff == NULL)
					return false;
				memcpy(ptmpbuff,chdata,ndatalen);
				psendstructx_ ptask = (psendstructx_)pchtmp;
				ptask->pbuff = ptmpbuff;
				ptask->uilen = ndatalen;
				m_sendqueuex.push_back(ptask);
			}
			if (m_bsend)
				return true;				
			else
			{
				psendstructx_ ptask = m_sendqueuex.pop_front();
				if (ptask == NULL)
					return false;
				m_chsendbuff = ptask->pbuff;
				m_uineedsend = ptask->uilen;
				m_uisended = 0;

				m_bsend = true;
				return overlapped_first_send();
			}
		}
		bool begin_send_task()
		{
			_lockx lock(&m_lockobject);
			m_bsend = false;
			psendstructx_ ptask = m_sendqueuex.pop_front();
			if (ptask == NULL)
				return false;
			m_chsendbuff = ptask->pbuff;
			m_uineedsend = ptask->uilen;
			m_uisended = 0;
			m_bsend = true;
			return true;
		}
		bool post_recv()
		{
			if (m_brecv == true)
				return false;
			
			DWORD   bytes,flags;
			int     rc;
			ZeroMemory(&(m_recvobjx.ol),sizeof(WSAOVERLAPPED));
			m_recvobjx.wbuf.buf = m_chrecvbuff + m_uirecved;
			m_recvobjx.wbuf.len = m_uirecvbufflen - m_uirecved;

			flags = 0;

			rc = WSARecv(
				m_ssocket,
				&m_recvobjx.wbuf,
				1,
				&bytes,
				&flags,
				&m_recvobjx.ol,
				NULL
				);

			if (rc == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{		
					cout<<"PostRecv: WSARecv* failed! :"<<WSAGetLastError()<<endl;           
					return false;
				}
			}
			m_brecv = true;
			return true;
		}
		bool onrecv(unsigned int uirecvlen)
		{
			bool bret = true;
			m_brecv = false;
			m_uirecved += uirecvlen;

			//处理m_nHanReadBytes   循环处理接收到的数据，可能会有多个报文
			unsigned int ineedprocessnum = m_uirecved;	//总共累计的读取字节
			unsigned int iprocessednum = 0;
			while (ineedprocessnum >= sizeof(BaseMsgHead))
			{
				PBaseMsgHead phead = (PBaseMsgHead)(m_chrecvbuff + iprocessednum);
				if (phead->uilen > m_uirecvbufflen || phead->uilen < sizeof(BaseMsgHead))
				{
					m_piocp->close_link(this);
					return false;
				}
				if (phead->uilen <= ineedprocessnum)
				{
					bret = m_piocp->handle_recv_msg(this,(char*)phead,phead->uilen);
					if (bret == false)
					{
						m_piocp->close_link(this);
						return false;
					}
					iprocessednum += phead->uilen;
					ineedprocessnum -= phead->uilen;
				}
				else
					break;
			}
			//移动剩下的
			m_uirecved = ineedprocessnum;
			if (m_uirecved > 0 && iprocessednum != 0)
				memcpy(m_chrecvbuff,m_chrecvbuff + iprocessednum,m_uirecved);

			m_dwlastalivetime = GetTickCount();
			return true;
		}
		bool onsend(unsigned int uisendlen)
		{
			m_dwlastalivetime = GetTickCount();
			m_uisended += uisendlen;
			if (m_uineedsend > m_uisended)
				return overlapped_next_send();
			else
			{
				if (begin_send_task())
					return overlapped_first_send();
			}
			return true;
		}
		inline int shut_down(int ihow = SD_BOTH){ return shutdown(m_ssocket,ihow); }
		inline int close_socket() { return closesocket(m_ssocket); }
	private:
		inline void clear_link()
		{
			m_dwSocketID = 0;
			m_ssocket = INVALID_SOCKET;
			m_bsend = false;
			m_brecv = false;
			m_uirecved = 0;
			m_uineedsend = 0;
			m_uisended = 0;
			m_::free(m_chsendbuff);
			m_chsendbuff = NULL;
			clear_queuex();
		}
		inline void clear_queuex()
		{
			psendstructx_ tmp = NULL;
			while (tmp = m_sendqueuex.pop_front())
			{
				m_::free(tmp->pbuff);
				m_::free(tmp);
			}
		}
	public:
		DWORD						m_dwSocketID;
		SOCKET						m_ssocket;
		DWORD						m_dwlastalivetime;	//该用户作为主机最后的KeepAlive时间
		
		DWORD						m_dwpeerip;
		unsigned short				m_usport;

		per_io_objx_				m_recvobjx;	
		per_io_objx_				m_sendobjx;

		bool						m_bsend;	//是否在send
		bool						m_brecv;	//是否在recv

		bool						m_bdelete;	//是否即将被删除
		iocpx_<m_>*					m_piocp;	

	private:
		
		ptrqueuex<sendstructx_,5,false>		m_sendqueuex;	//发送缓冲区
			
		char*						m_chrecvbuff;	//接收缓冲区
		unsigned int				m_uirecvbufflen;	//缓冲区大小
		unsigned int				m_uirecved;		//缓冲区里已有数据长度
		
		char*						m_chsendbuff;
		unsigned int				m_uisended;			//当前已经发送的大小
		unsigned int				m_uineedsend;   	//需要发送的大小

		criticalsectionx_			m_lockobject;

		static const unsigned int	g_uipersendsize = 10 * 1024;
	};

};
#endif	//LINKX_NET_XHB_H_