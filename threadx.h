#ifndef THREADX_SYSTEM_XHB_H_
#define THREADX_SYSTEM_XHB_H_

#include <process.h>
#include "windows.h"
namespace xhb
{
	class threadx_
	{
	public:
		threadx_():m_hThread(0),m_dwThreadID(0){}
		virtual ~threadx_()
		{
			if (m_hThread)
			{
				CloseHandle(m_hThread);
				m_hThread = NULL;
			}
			m_dwThreadID = 0;
		}
		virtual bool InitInstance()
		{
			//do prepare work;
			return true;
		}
		virtual void MainFunction()
		{
			int i = 1;
			while(true)
			{
				//do things
				printf("%d\n",++i);
			}
		}
		virtual unsigned int ExitInstance()
		{
			//do exit thing
			return 0;
		}
		bool Start()
		{
			m_hThread = (HANDLE)_beginthreadex(NULL, 0, threadproc,this, CREATE_SUSPENDED, (UINT*)&m_dwThreadID);
			if( m_hThread == NULL ) 
				return false;
			return ::ResumeThread(m_hThread) != (DWORD) -1;
		}
		bool Terminate(DWORD dwExitCode = 0) const
		{
			return TRUE == ::TerminateThread(m_hThread, dwExitCode);
		}
		bool WaitForThread(DWORD dwTimeout = INFINITE) const
		{
			return ::WaitForSingleObject(m_hThread, dwTimeout) == WAIT_OBJECT_0;
		}
		bool SetPriority(int iPriority) const
		{
			return TRUE == ::SetThreadPriority(m_hThread, iPriority);
		}

		int GetPriority() const
		{
			return ::GetThreadPriority(m_hThread);
		}
		bool Suspend()
		{
			if( ::SuspendThread(m_hThread) == (DWORD) -1 ) 
				return false;
			return true;
		}

		bool Resume()
		{
			if( ::ResumeThread(m_hThread) == (DWORD) -1 ) 
				return false;
			return true;
		}
		bool IsRunning() const
		{
			if( m_hThread == NULL ) 
				return false;
			DWORD dwCode = 0;
			::GetExitCodeThread(m_hThread, &dwCode);
			return dwCode == STILL_ACTIVE;
		}
		DWORD GetThreadID() const
		{
			return m_dwThreadID;
		}
		operator HANDLE() const 
		{ 
			return m_hThread; 
		}
	private:
		static unsigned int WINAPI threadproc(void* pPara)
		{
			threadx_* pthread = (threadx_*) pPara;
			unsigned int uiret = pthread->Run();
			return uiret;
		}
		unsigned int Run()
		{
			if (!InitInstance())
			{
				ExitInstance();
				return 1;
			}	
			MainFunction();
			ExitInstance();
			return 0;
		}
	private:
		HANDLE	m_hThread;
		DWORD	m_dwThreadID;
	};
};
#endif	//THREADX_SYSTEM_XHB_H_