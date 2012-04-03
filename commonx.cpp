#include "commonx.h"
#pragma warning(push)
#pragma warning(disable : 4800)	//去掉char转bool类型警告
#pragma warning(disable : 4996)	//去掉安全函数警告
#pragma warning(disable : 4018)	//去掉signed 和 unsigned转换警告

namespace xhb
{
	bool basefunctionx_::m_bsmallbyteorder = false;
	char basefunctionx_::m_programpath[256] = {0};

	bool basefunctionx_::initiate()
	{
		unsigned short ustest = 1;
		m_bsmallbyteorder= (*(char*)(&ustest));

		int j = 0;
		GetModuleFileName(0, m_programpath, sizeof(m_programpath));

		for(j = (int)strlen(m_programpath); j > 0; j--)
			if(m_programpath[j] == '\\')
				break;
		m_programpath[j] = 0;			

		return true;
	}
	void basefunctionx_::get_format_time( const char* pstrformat,char* pstrret,unsigned int nretlen)
	{
		assert(pstrformat != NULL && pstrret !=NULL);
		time_t ltime;
		struct tm *gmt;
		char timebuf[256];
		memset( timebuf, 0, sizeof( timebuf ));
		time(&ltime);
		gmt = localtime(&ltime);
		strftime( timebuf, sizeof( timebuf ),pstrformat, gmt );
		memcpy(pstrret,timebuf,min(strlen(timebuf) + 1,nretlen));	
	}
	unsigned long basefunctionx_::get_ip_value(const char* pstrip)
	{
		assert(pstrip != NULL);
		return inet_addr(pstrip);
	}
	string basefunctionx_::get_ip_str(unsigned long ulip)
	{
		char chtmp[32];
		if (m_bsmallbyteorder)
			sprintf(chtmp,"%d.%d.%d.%d",ulip&0x000000ff,(ulip>>8)&0x000000ff,(ulip>>16)&0x000000ff,ulip>>24);	//小头
		else
			sprintf(chtmp,"%d.%d.%d.%d",ulip>>24,(ulip>>16)&0x000000ff,(ulip>>8)&0x000000ff,ulip&0x000000ff);	//大头
		return chtmp;
	}
	void basefunctionx_::get_ip_str(unsigned long ulip,char* buff)
	{
		assert(buff != NULL);
		if(m_bsmallbyteorder)
			sprintf(buff,"%d.%d.%d.%d",ulip&0x000000ff,(ulip>>8)&0x000000ff,(ulip>>16)&0x000000ff,ulip>>24);	//小头
		else
			sprintf(buff,"%d.%d.%d.%d",ulip>>24,(ulip>>16)&0x000000ff,(ulip>>8)&0x000000ff,ulip&0x000000ff);	//大头
	}
	int basefunctionx_::ws2s(const wchar_t* pwstrin,char* pstrout,unsigned int uisize)
	{
		assert(pwstrin != NULL && pwstrin != NULL);
		int nlen = WideCharToMultiByte(CP_ACP, NULL, pwstrin, -1, NULL, 0, NULL, NULL);
		if (uisize < nlen) 
			return nlen;

		WideCharToMultiByte(CP_ACP, 0, pwstrin, -1, pstrout, nlen, NULL, NULL);
		pstrout[nlen -1] = 0;
		return 0;
	}
	string basefunctionx_::ws2s(const wstring& wstr)
	{
		unsigned int nlen = wstr.length() * 2 + 2;
		char *ptmpbuff = ::new(std::nothrow) char[nlen];
		if (ptmpbuff == NULL)
			return "";
		int nret = ws2s(wstr.c_str(),ptmpbuff,nlen);
		assert(nret == 0);
		string strtmp(ptmpbuff);
		delete [] ptmpbuff;
		return strtmp;
	}
	int basefunctionx_::s2ws(const char* pstrin,wchar_t* pwstrout,unsigned int uisize)
	{
		assert(pwstrout != NULL && pstrin != NULL);
		int nsize = MultiByteToWideChar(CP_ACP, 0, pstrin, strlen(pstrin), 0, 0);

		if(uisize < nsize) 
			return nsize;

		MultiByteToWideChar(CP_ACP, 0,pstrin,strlen(pstrin), pwstrout, nsize);
		pwstrout[nsize - 1] = 0;

		if(pwstrout[0] == 0xfeff) // skip Oxfeff
			memmove(pwstrout,pwstrout + 1,(nsize - 1) * 2);
		return 0;
	}
	wstring basefunctionx_::s2ws(const string& str)
	{
		unsigned int nlen = str.length() + 1;
		wchar_t *ptmpbuff = ::new(std::nothrow) wchar_t[nlen];
		if (ptmpbuff == NULL)
			return L"";
		int nret = s2ws(str.c_str(),ptmpbuff,nlen);
		assert(nret == 0);
		wstring wstrtmp(ptmpbuff);
		delete [] ptmpbuff;
		return wstrtmp;
	}
};

#pragma warning(pop)
