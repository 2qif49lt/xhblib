#pragma once

class CMyLock
{
public:
	CMyLock(void);
	~CMyLock(void);
public:
	void Lock();
	void Unlock();
	//bool TryLock();
private:
	CRITICAL_SECTION  m_csLock;
};


class CMyAutoLock
{
public:
	CMyAutoLock(CMyLock& lock);
	~CMyAutoLock();
private:
	CMyLock& m_lock;
};


class CMyAutoLock2
{
public:
	CMyAutoLock2(CMyLock* pCSLock);
	~CMyAutoLock2();
private:
	CMyLock* m_pCSLock;
};


class CMyRWLock
{
public:
	CMyRWLock();
	~CMyRWLock();
public:
	void  ReadLock();
	void  UnReadLock();
	void  WriteLock();
	void  UnWriteLock();
private:
	volatile long  m_ulReadCount;
	CRITICAL_SECTION  m_csLock;
};

class CMyAutoReadLock
{
public:
	CMyAutoReadLock(CMyRWLock& lock);
	~CMyAutoReadLock();
private:
	CMyRWLock& m_ReadLock;
};

class CMyAutoWriteLock
{
public:
	CMyAutoWriteLock(CMyRWLock& lock);
	~CMyAutoWriteLock();
private:
	CMyRWLock& m_WriteLock;
};

#define MY_AUTOLOCK(lock) CMyAutoLock aLock(lock);
#define MY_AUTOLOCK2(lock) CMyAutoLock2 aLock(&lock);
#define MY_AUTOLOCK_READ(lock) CMyAutoReadLock aLock(lock);
#define MY_AUTOLOCK_WRITE(lock) CMyAutoWriteLock aLock(lock);

#define RWLOCK_FREE   0
#define RWLOCK_LOCKED  1

class CRWLock
{
public:
	inline CRWLock(void):m_ulRLock(0),m_ulWLock(0),m_ulPreLock(0){};
	void WLock(void);
	void RLock(void);

	inline void WUnLock(void)
	{
		InterlockedExchange((LPLONG)&m_ulWLock, RWLOCK_FREE);
	}

	inline void RUnLock(void)
	{
		if (m_ulRLock>0)
		{
			InterlockedDecrement((LPLONG)&m_ulRLock);
		}
	}

	inline void Lock(void){WLock();}
	inline void UnLock(void){WUnLock();}
private:
	UINT32 volatile m_ulWLock;
	UINT32 volatile m_ulRLock;
	UINT32 volatile m_ulPreLock;
};
