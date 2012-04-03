#ifndef LISTENTHREADX_NET_XHB_H_
#define LISTENTHREADX_NET_XHB_H_
#include "memoryx.h"
#include "threadx.h"
#include "linkx.h"
#include "iocpx.h"
namespace xhb
{
	template<typename m_>
	class iocpx_;

	template<typename m_ = mm_char_defaultx_>
	class listenthreadx : public threadx_
	{
	public:
		listenthreadx(){m_socketid = 0;}
		~listenthreadx(){}
	public:
		virtual bool InitInstance()
		{
			return threadx_::InitInstance();
		}
		virtual void MainFunction();
		virtual unsigned int ExitInstance()
		{
			return threadx_::ExitInstance();
		}
	public:
		unsigned short			m_sPort;	//Listen 端口号
		iocpx_<m_>*				m_piocpx;
	private:
		void			onaccept();
		WSAEVENT		m_hevent;
		SOCKET			m_slisten;
		unsigned long	m_socketid;
	};
	template<typename m_>
	void listenthreadx<m_>::MainFunction()
	{
		WSADATA wsaData;
		DWORD Ret;


		if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
			return ;

		//创建Listen socket
		if ((m_slisten = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
			return;

		// Event for handling Network IO
		m_hevent = WSACreateEvent();
		if (m_hevent == WSA_INVALID_EVENT)
		{
			closesocket(m_slisten);
			return;
		}

		// Request async notification
		int nRet = WSAEventSelect(m_slisten, m_hevent, FD_ACCEPT);

		if (nRet == SOCKET_ERROR)
		{
			closesocket(m_slisten);
			return;
		}

		SOCKADDR_IN InternetAddr;
		InternetAddr.sin_family = AF_INET;
		InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		InternetAddr.sin_port = htons(m_sPort);

		if (bind(m_slisten, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
		{
			m_piocpx->m_brunning = false;		//程序需要退出 
			return;
		}

		// Set the socket to listen
		nRet = listen(m_slisten, 100);
		if (nRet == SOCKET_ERROR)
		{
			closesocket(m_slisten);
			m_piocpx->m_brunning = false;		//程序需要退出
			return;
		}

		//Listen
		WSANETWORKEVENTS events;
		while(m_piocpx->m_brunning)
		{

			DWORD dwRet;
			dwRet = WSAWaitForMultipleEvents(1,&m_hevent, FALSE, 1000, FALSE);

			if (dwRet == WSA_WAIT_TIMEOUT)
				continue;

			int nRet = WSAEnumNetworkEvents(m_slisten, m_hevent, &events);

			if (nRet == SOCKET_ERROR)
				break;

			if (events.lNetworkEvents & FD_ACCEPT)
			{
				if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
					onaccept();
				else	
					break;
			}
		}
		
		closesocket(m_slisten);
		//线程就退出了
	}
	template<typename m_>
	void listenthreadx<m_>::onaccept()
	{
		SOCKADDR_IN	SockAddr;
		SOCKET		clientSocket;

		int			nRet;
		int			nLen;

		nLen = sizeof(SOCKADDR_IN);

		clientSocket = WSAAccept(m_slisten, (LPSOCKADDR)&SockAddr,&nLen,NULL,0); 

		if (clientSocket == INVALID_SOCKET )
		{
			nRet = WSAGetLastError();
			if (nRet != WSAEWOULDBLOCK)
				return;
		}
		int nSockSendBufferLen = 30 * 1024;
		int nSockRecvBufferLen = 30 * 1024;
		setsockopt(clientSocket,SOL_SOCKET,SO_SNDBUF,(char*)&nSockSendBufferLen,sizeof(int));
		setsockopt(clientSocket,SOL_SOCKET,SO_RCVBUF,(char*)&nSockRecvBufferLen,sizeof(int));
		tcplinkx_<m_>* plink = m_::alloc<tcplinkx_<m_> >();

		if(plink ==NULL)
			return;

		plink->m_dwSocketID = ++m_socketid;
		plink->m_ssocket = clientSocket;	
		plink->m_dwpeerip = SockAddr.sin_addr.S_un.S_addr;
		plink->m_usport = SockAddr.sin_port;
		plink->m_piocp = m_piocpx;
		if (plink->init_buff(m_piocpx->m_uirecvbufflen) == false)
		{
			m_::free(plink);
			closesocket( clientSocket );
			return;
		}
		
		if(CreateIoCompletionPort((HANDLE) plink->m_ssocket , m_piocpx->m_hCompletionPort, (DWORD)plink, 0)==0)
		{	
			m_::free(plink);
			closesocket( clientSocket );
			return;
		}

		plink->m_recvobjx.operation = OP_ACCEPT;

		BOOL bSuccess = PostQueuedCompletionStatus(m_piocpx->m_hCompletionPort, -1, (DWORD)plink, &(plink->m_recvobjx.ol));

		if ( (!bSuccess ))
		{            
			m_::free(plink);
			closesocket( clientSocket );
			return;
		}	
	}
};
#endif	//LISTENTHREADX_NET_XHB_H_