#ifndef COMMON_LANGUAGE_XHB_H_
#define COMMON_LANGUAGE_XHB_H_

/************************************************************************/
/* 

1.经常共用的类,函数,宏定义,等等在这里 

注意:函数返回值是string之类的话,可能引起dll栈破坏
*/
/************************************************************************/

#include "windows.h"

#ifndef NULL
#define NULL    0
#endif
#include <assert.h>
#include "time.h"
#include <new>
#include <string>
using std::string;
using std::wstring;

namespace xhb
{
	
	//获取数组元素个数
	template<typename T,int n>
	int sizeofx(T (&arr)[n])
	{
		return n;
	}
	
	//简单2进制
	template<int n>
	struct binaryx_
	{
		enum{value = n % 10 + binaryx_<n / 10>::value * 2};
	};
	template<>
	struct binaryx_<0>
	{
		enum{value = 0};
	};

	class basefunctionx_
	{
	public:
		//要先调用这个函数.
		static bool initiate();

		//获取格式化的时间 格式为："%Y-%m-%d %H:%M:%S"
		inline static void get_format_time( const char* pstrformat,char* pstrret,unsigned int nretlen);

		//获得数值型的ip
		inline static unsigned long get_ip_value(const char* pstrip);

		//获得std::string的ip,inet_ntoa 线程不安全
		inline static string get_ip_str(unsigned long ulip);

		//外部调用保证buff 够长.
		inline static void get_ip_str(unsigned long ulip,char* buff);

		//获取当前程序运行的路径:"F:\folder"
		inline static const char*  get_program_path(){ return m_programpath;}

		//转宽字符为多字节，uisize为out缓冲区元素大小个数,函数返回0表示成功,非0表示失败和需要多少char个数缓冲区
		static int ws2s(const wchar_t* pwstrin,char* pstrout,unsigned int uisize);
		//简单接口,内部申请内存
		static string ws2s(const wstring& wstr);
		//转多字节为宽字符，uisize为out缓冲区元素大小个数,函数返回0表示成功,非0表示失败和需要多少wchar_t个数缓冲区
		static int s2ws(const char* pstrin,wchar_t* pwstrout,unsigned int uisize);
		//简单接口,内部申请内存
		static wstring s2ws(const string& str);
	private:
		basefunctionx_(){}
	private:
		static bool m_bsmallbyteorder;
		static char m_programpath[256];
	};

	
}

#endif	//COMMON_LANGUAGE_XHB_H_