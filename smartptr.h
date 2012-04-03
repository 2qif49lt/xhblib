#ifndef SMARTPTR_LANGUAGE_XHB_H_
#define SMARTPTR_LANGUAGE_XHB_H_
/************************************************************************/
/* 
引用技术智能指针：有2个
1，inptrx_ 是侵入式引用计数,需要将refcountx_继承为基类（这样不支持基础类型）
2，smptrx_ 是自带引用计数，支持基础类型

支持外界内存管理,以避免计数器的new delete小内存碎片。
默认是采用new和delete.
如果想使用内存池 请编写和memmanger_的相同的接口类

简单使用方式：
class aaa : public refcountx_<aaa>
{
public:
int i ;
aaa( int n )
{
i = n;
}
aaa& operator= (int n){i = n;return *this;}
protected:
private:
};

int main(int argc, char **argv)
{
{
int *p = new int;
*p = 123;
smptrx_<int> sp ;
sp = p;
cout<<*sp<<endl;
}
{
inptrx_<aaa> a = new aaa(101);
cout<<a->i<<endl;
}
system("pause");
return 0;   // Program successfully completed.
}

*/
/************************************************************************/
#include "memoryx.h"
namespace xhb
{

	//侵入式引用技术类,所有数据类的基类.(不支持基础类型)
	template<typename t_,
		template<typename s_> class m_ = mm_class_defaultx_>
	class refcountx_
	{
	private:
		int m_nref;
	public:
		refcountx_() : m_nref(0) {}
		virtual ~refcountx_() {}
		virtual void upcount() { ++m_nref; }
		virtual void downcount(void)
		{
			if (--m_nref == 0)
			{
				m_<t_>::free(dynamic_cast<t_*>(this));
			}
		}
		virtual bool lessthan(const refcountx_* other)
		{
			return this < other;
		}
	};

	//侵入式引用计数智能指针类.
	template <typename t_> 
	class inptrx_
	{
	private:
		t_* m_ptr;

	public:
		typedef inptrx_<t_> mytypex_;

	public:
		inptrx_(const mytypex_& sptr) : m_ptr(sptr.m_ptr) { if (m_ptr) m_ptr->upcount(); }
		inptrx_() : m_ptr(0) {}
		inptrx_(t_* ptr) : m_ptr(ptr) { if (m_ptr) m_ptr->upcount(); }
		~inptrx_() { if (m_ptr) m_ptr->downcount(); }

		operator t_*() const { return m_ptr; }
		t_& operator*() const { return *m_ptr; }
		t_* operator->() const { return m_ptr; }

		mytypex_& operator=(t_* ptr)
		{
			if (m_ptr) m_ptr->downcount();
			m_ptr = ptr;
			if (m_ptr) m_ptr->upcount();
			return *this;
		}
		mytypex_& operator=(const t_* ptr) { return operator=((t_*)ptr); }
		mytypex_& operator=(const mytypex_ &sptr) { return operator=((t_*)(mytypex_)sptr); }		
		bool operator ==(const mytypex_ &sptr) const { return m_ptr == sptr.m_ptr; }
		bool operator ==(const t_* ptr) const { return m_ptr == ptr; }
		bool operator !=(const mytypex_ &sptr) const { return m_ptr != sptr.m_ptr; }
		bool operator !=(const t_* ptr) const { return m_ptr != ptr; }
		bool operator < (const mytypex_ &sptr) const { return m_ptr->lessthan(sptr.m_ptr); }
	};


	//内置计数器的引用计数智能指针
	template<typename t_,
		template<typename s_> class m_ = mm_class_defaultx_>
	class smptrx_
	{
	private:
		t_* m_ptr;
		int* m_nref;
	public:
		typedef smptrx_<t_,m_> mytypex_;

	public:
		smptrx_(const mytypex_& sptr) : m_ptr(sptr.m_ptr),m_nref(sptr.m_nref) 
		{ if (m_ptr) upcount(); }
		smptrx_() : m_ptr(0),m_nref(0){}
		smptrx_(t_* ptr) : m_ptr(ptr),m_nref(0) { if (m_ptr) upcount(); }
		~smptrx_() { if (m_ptr) downcount(); }

		operator t_*() const { return m_ptr; }
		t_& operator*() const { return *m_ptr; }
		t_* operator->() const { return m_ptr; }

		mytypex_& operator=(t_* ptr)
		{
			if (m_ptr) downcount();
			m_ptr = ptr;
			if (m_ptr) upcount();
			return *this;
		}
		mytypex_& operator=(const t_* ptr) { return operator=((t_*)ptr); }
		mytypex_& operator=(const mytypex_ &sptr) { return operator=((t_*)(mytypex_)sptr); }		
		bool operator ==(const mytypex_ &sptr) const { return m_ptr == sptr.m_ptr; }
		bool operator ==(const t_* ptr) const { return m_ptr == ptr; }
		bool operator !=(const mytypex_ &sptr) const { return m_ptr != sptr.m_ptr; }
		bool operator !=(const t_* ptr) const { return m_ptr != ptr; }
		bool operator < (const mytypex_ &sptr) const { return m_ptr->lessthan(sptr.m_ptr); }
	
	private:
		void upcount() 
		{ 
			if(m_nref == 0)
			{
				m_nref = m_<int>::alloc();
				*m_nref = 0;
			}
			++(*m_nref); 
		}
		void downcount(void)
		{
			if (m_nref && (--(*m_nref) == 0))
			{
				m_<t_> ::free(m_ptr);
				m_<int>::free(m_nref);
			}
		}
	};
};


#endif //SMARTPTR_LANGUAGE_XHB_H_