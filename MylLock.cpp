#include "StdAfx.h"
#include "Mylock.h"

CMyLock::CMyLock(void)
{
	::InitializeCriticalSection(&m_csLock);
}

CMyLock::~CMyLock(void)
{
	::DeleteCriticalSection(&m_csLock);
}

void CMyLock::Lock()
{
	::EnterCriticalSection(&m_csLock);
}

void CMyLock::Unlock()
{
	::LeaveCriticalSection(&m_csLock);
}

//bool CMyLock::TryLock()
//{
//	return ::TryEnterCriticalSection(&m_csLock);
//};

////////////////////////////////////////////////////////////////////////////////
CMyAutoLock::CMyAutoLock(CMyLock &lock):m_lock(lock)
{
	m_lock.Lock();
}
CMyAutoLock::~CMyAutoLock()
{
	m_lock.Unlock();
}

////////////////////////////////////////////////////////////////////////////////
CMyAutoLock2::CMyAutoLock2(CMyLock* pCSLock)
{
	m_pCSLock = NULL;
	if(pCSLock)
	{
		m_pCSLock = pCSLock;
		m_pCSLock->Lock();		
	}
}
CMyAutoLock2::~CMyAutoLock2()
{
	if (m_pCSLock)
	{
		m_pCSLock->Unlock();
	}
}

////////////////////////////////////////////////////////////////////////////////
CMyRWLock::CMyRWLock()
:m_ulReadCount(0)
{
	::InitializeCriticalSection(&m_csLock);
}
CMyRWLock::~CMyRWLock()
{
	::DeleteCriticalSection(&m_csLock);
}
void  CMyRWLock::ReadLock()
{
	::EnterCriticalSection(&m_csLock);
	::InterlockedIncrement(&m_ulReadCount);
	::LeaveCriticalSection(&m_csLock);

}
void  CMyRWLock::UnReadLock()
{
	::InterlockedDecrement(&m_ulReadCount);
}
void  CMyRWLock::WriteLock()
{
	::EnterCriticalSection(&m_csLock);
	while(m_ulReadCount>0)
	{
		Sleep(0);
	}
}
void  CMyRWLock::UnWriteLock()
{
	::LeaveCriticalSection(&m_csLock);
}

////////////////////////////////////////////////////////////////////////////////
CMyAutoReadLock::CMyAutoReadLock(CMyRWLock& lock) : m_ReadLock(lock)
{
	m_ReadLock.ReadLock();
}

CMyAutoReadLock::~CMyAutoReadLock()
{
	m_ReadLock.UnReadLock();
}

////////////////////////////////////////////////////////////////////////////////
CMyAutoWriteLock::CMyAutoWriteLock(CMyRWLock& lock) : m_WriteLock(lock)
{
	m_WriteLock.WriteLock();
}
CMyAutoWriteLock::~CMyAutoWriteLock()
{
	m_WriteLock.UnWriteLock();
}

////////////////////////////////////////////////////////////////////////////////
void CRWLock::WLock(void)
{
	while (InterlockedCompareExchange((LONG*)&m_ulPreLock, (LONG)RWLOCK_LOCKED,
		(LONG)RWLOCK_FREE) != 0)
	{
		Sleep(0);
	}

	while (InterlockedCompareExchange((LONG*)&m_ulRLock, (LONG)RWLOCK_FREE, 
		(LONG)RWLOCK_FREE) != 0)
	{
		Sleep(0);
	}

	while (InterlockedCompareExchange((LONG*)&m_ulWLock, (LONG)RWLOCK_LOCKED, 
		(LONG)RWLOCK_FREE) != 0)
	{
		Sleep(0);
	}

	InterlockedExchange((LPLONG)&m_ulPreLock, RWLOCK_FREE);
}

void CRWLock::RLock(void)
{
	while (InterlockedCompareExchange((LONG*)&m_ulPreLock, (LONG)RWLOCK_LOCKED, 
		(LONG)RWLOCK_FREE) != 0)
	{
		Sleep(0);
	}

	while (InterlockedCompareExchange((LONG*)&m_ulWLock, (LONG)RWLOCK_FREE, 
		(LONG)RWLOCK_FREE) != 0)
	{
		Sleep(0);
	}

	InterlockedIncrement((LPLONG)&m_ulRLock);

	InterlockedExchange((LPLONG)&m_ulPreLock, RWLOCK_FREE);
}