#pragma once
#include <string>
#include <windows.h>

class CAutoTimer
{
public:
	CAutoTimer(const char* szName):m_strName(szName) 
	{
		if( !QueryPerformanceFrequency (&m_nFrequency) )
		{
			m_nBegin.LowPart = GetTickCount();
			m_bSupport = FALSE;
		}
		else
		{
			QueryPerformanceCounter(&m_nBegin);
			m_bSupport = TRUE;;
		}
	}

	~CAutoTimer() 
	{
		if( !m_bSupport )
		{
			DWORD dwDelta = GetTickCount() - m_nBegin.LowPart;
			printf("%s: %u (ms)\n", m_strName.c_str(), dwDelta);
		}
		else
		{
			LARGE_INTEGER end;
			QueryPerformanceCounter(&end);
			double fDelta = (double)(end.QuadPart - m_nBegin.QuadPart) * 1000 / m_nFrequency.QuadPart;
			printf("%s: %f (ms)\n", m_strName.c_str(), fDelta);
		}
	}
private:
	std::string m_strName;
	LARGE_INTEGER m_nFrequency;
	LARGE_INTEGER m_nBegin;
	BOOL m_bSupport;
};

#define SET_AUTO_TIMER(name) CAutoTimer __CAutoTimer##name(#name)