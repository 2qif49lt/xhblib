#ifndef LOCKX_SYSTEM_XHB_H_
#define LOCKX_SYSTEM_XHB_H_
/************************************************************************/
/* 
临界区 锁
使用方式
criticalsectionx_ m_lockobject;
{
lockx_<criticalsectionx_> lock(&m_lockobject);
}

falselockx_ falseobject;
{
lockx_<falselockx_> lock(&falseobject);
}
*/
/************************************************************************/
#ifdef _WIN32
#include "Windows.h"
#else
#include <pthread.h>
#endif

namespace xhb
{
	
	//假锁
	class falselockx_
	{
	public:
		void lock() {}
		void unlock(){}
	};

	class criticalsectionx_
	{
	private:
		CRITICAL_SECTION m_sec;

	public:
		criticalsectionx_() 
		{
			::InitializeCriticalSection(&m_sec);
		}

		~criticalsectionx_() 
		{
			::DeleteCriticalSection(&m_sec);
		}

	public:
		void lock() 
		{
			::EnterCriticalSection(&m_sec);
		}

		void unlock() 
		{
			::LeaveCriticalSection(&m_sec);
		}
        bool trylock()
        {
            return ::TryEnterCriticalSection(&m_sec) != 0;
        }
		operator CRITICAL_SECTION&()()
		{
			return m_sec;
		}
	};
    class pthreadmutex_
    {
    public:
        pthreadmutex_()
        {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&m_mutex,&attr);
        }
        ~pthreadmutex_()
        {
            pthread_mutex_destroy(&m_mutex);
        }
    public:
        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&m_mutex);
        }
        bool trylock()
        {
            return pthread_mutex_trylock(&m_mutex) == 0;
        }
        operator pthread_mutex_t&()()
        {
            return m_mutex;
        }
    private:
        pthread_mutex_t m_mutex;
    };
	template<typename l_>
	class lockx_
	{
	public:
		lockx_(l_* plockobject)
		{
			m_plock = plockobject;
			m_plock->lock();
		}
		~lockx_()
		{
			m_plock->unlock();
		}
	private:
		lockx_(){};
		l_* m_plock;
	};
	typedef lockx_<criticalsectionx_>  _lockx;
}
#endif	//LOCKX_SYSTEM_XHB_H_