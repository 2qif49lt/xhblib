#ifndef WORKERTHREADX_NET_XHB_
#define WORKERTHREADX_NET_XHB_

#include "memoryx.h"
#include "threadx.h"
#include "linkx.h"
#include "iocpx.h"
namespace xhb
{
	template<typename m_>
	class iocpx_;

	template<typename m_ = mm_char_defaultx_>
	class workerthreadx : public threadx_
	{
	public:
		workerthreadx(){}
		~workerthreadx(){}
	public:
		virtual bool InitInstance();
		virtual void MainFunction();
		virtual unsigned int ExitInstance()
		{
			return threadx_::ExitInstance();
		}
	public:
		iocpx_<m_>*				m_piocpx;
	};
	template<typename m_>
	bool workerthreadx<m_>::InitInstance()
	{
		DWORD     Ret;
		WSADATA   wsaData;

		if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
			return false;
		return threadx_::InitInstance();
	}
	template<typename m_>
	void workerthreadx<m_>::MainFunction()
	{
		int nRet;
		int PrintDebugInfo_Flag=0;

		DWORD			 dwCumId;
		OVERLAPPED      *Overlap;
		pper_io_objx_	pverlapobject;

		DWORD           dwBytesXfered;
		HANDLE			g_CompletionPort=m_piocpx->m_hCompletionPort;

		DWORD dwMilliseconds = 1000;
		while(m_piocpx->m_brunning)
		{		
			nRet= GetQueuedCompletionStatus(g_CompletionPort,&dwBytesXfered,(unsigned long *)&dwCumId,&Overlap,dwMilliseconds);
			if(nRet==0 && Overlap==NULL)	//超时
				continue;

			pverlapobject = (pper_io_objx_)Overlap;	
			m_piocpx->handle_complate_notify(dwBytesXfered,(tcplinkx_<m_>*)dwCumId,pverlapobject);
		}	
	}
};
#endif	//WORKERTHREADX_NET_XHB_