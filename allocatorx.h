#ifndef ALLOCATOR_LANGUAGE_XHB_H_
#define ALLOCATOR_LANGUAGE_XHB_H_

/************************************************************************/
/* 
stl分配器类
可以外接内存池管理内存,避免msstl分配器产生内存碎片
使用方法

#include "allocatorx.h"
#include "memory_pool.h"
using xhb::memory_pool_xhb;
class AA
{
public:
AA(){str = "hello!";}
string str;
};
memory_pool_xhb g_mempool;
class mempoolinterface
{
public:
inline static void* alloc(size_t ncount)
{
return (void*)g_mempool.alloc(ncount);
}
inline static void free(void* ptr)
{
return g_mempool.free((char*)ptr);
}
};
int main()
{

{
vector<AA,allocatorx_<AA> > vec;
AA a;
vec.push_back(a);
}
{
vector<AA,allocatorx_<AA,mempoolinterface> > vec;
AA a;
vec.push_back(a);
}	
system("pause");
return 0;
}

int i = 0;
vector<int,allocatorx_<int,mempoolinterface> > vec;
while (i < 1000)
{
vec.push_back(i);
++i;
}
typedef vector<int,allocatorx_<int,mempoolinterface> > vectorx;
for (vectorx::iterator iter = vec.begin(); iter != vec.end(); ++iter)
{
cout<<*iter<<endl;
}
*/
/************************************************************************/
#include "memoryx.h"
namespace xhb
{

#define _DESTRUCTORX(ty, ptr)	(ptr)->~ty()

#ifndef _FARQ	/* specify standard memory model */
#define _FARQ
#define _PDFT	ptrdiff_t
#define _SIZT	size_t
#endif /* _FARQ */

	template<class _Ty> inline
		_Ty _FARQ *_Allocatex(_SIZT _Count, _Ty _FARQ *)
	{	// check for integer overflow
		if (_Count <= 0)
			_Count = 0;
		else if (((_SIZT)(-1) / _Count) < sizeof (_Ty))
			_THROW_NCEE(std::bad_alloc, NULL);

		// allocate storage for _Count elements of type _Ty
		return ((_Ty _FARQ *)::operator new(_Count * sizeof (_Ty)));
	}

	// TEMPLATE FUNCTION _Construct
	template<class _T1,
	class _T2> inline
		void _Constructx(_T1 _FARQ *_Ptr, const _T2& _Val)
	{	// construct object at _Ptr with value _Val
		void _FARQ *_Vptr = _Ptr;
		::new (_Vptr) _T1(_Val);
	}

	// TEMPLATE FUNCTION _Destroy
	template<class _Ty> inline
		void _Destroyx(_Ty _FARQ *_Ptr)
	{	// destroy object at _Ptr
		_DESTRUCTORX(_Ty, _Ptr);
	}

	template<> inline
		void _Destroyx(char _FARQ *)
	{	// destroy a char (do nothing)
	}

	template<> inline
		void _Destroyx(wchar_t _FARQ *)
	{	// destroy a wchar_t (do nothing)
	}

	// TEMPLATE CLASS _Allocator_base
	template<class _Ty>
	struct _Allocator_basex_
	{	// base class for generic allocators
		typedef _Ty value_type;
	};

	// TEMPLATE CLASS _Allocator_base<const _Ty>
	template<class _Ty>
	struct _Allocator_basex_<const _Ty>
	{	// base class for generic allocators for const _Ty
		typedef _Ty value_type;
	};



	// TEMPLATE CLASS allocator
	template<typename t_,typename m_ = mm_char_defaultx_>
	class allocatorx_
		: public _Allocator_basex_<t_>
	{	// generic allocator for objects of class t_
	public:
		typedef _Allocator_base<t_> _Mybase;
		typedef typename _Mybase::value_type value_type;
		typedef value_type  *pointer;
		typedef value_type  &reference;
		typedef const value_type  *const_pointer;
		typedef const value_type  &const_reference;

		typedef _SIZT size_type;
		typedef _PDFT difference_type;

		template<typename _Other>
		struct rebind
		{	// convert an allocator<t_> to an allocator <_Other>
			typedef allocatorx_<_Other,m_> other;
		};

		pointer address(reference _Val) const
		{	// return address of mutable _Val
			return (&_Val);
		}

		const_pointer address(const_reference _Val) const
		{	// return address of nonmutable _Val
			return (&_Val);
		}

		allocatorx_() 
		{	// construct default allocator (do nothing)
		}

		allocatorx_(const allocatorx_<t_,m_>&) _THROW0()
		{	// construct by copying (do nothing)
		}

		template<typename _Other>
		allocatorx_(const allocatorx_<_Other,m_>&) _THROW0()
		{	// construct from a related allocator (do nothing)
		}

		template<class _Other>
		allocatorx_<t_,m_>& operator=(const allocatorx_<_Other,m_>&)
		{	// assign from a related allocator (do nothing)
			return (*this);
		}

		void deallocate(pointer _Ptr, size_type)
		{	// deallocate object at _Ptr, ignore size
			m_::free((char*)_Ptr);
		}

		pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
			return (pointer)m_::alloc(_Count * sizeof(t_));
		}

		pointer allocate(size_type _Count, const void _FARQ *)
		{	// allocate array of _Count elements, ignore hint
			return (allocate(_Count));
		}

		void construct(pointer _Ptr, const t_& _Val)
		{	// construct object at _Ptr with value _Val
			_Constructx(_Ptr, _Val);
		}

		void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
			_Destroyx(_Ptr);
		}

		_SIZT max_size() const _THROW0()
		{	// estimate maximum array size
			_SIZT _Count = (_SIZT)(-1) / sizeof (t_);
			return (0 < _Count ? _Count : 1);
		}
	};

	// allocator TEMPLATE OPERATORS
	template<typename t_,typename m_,typename _Other>
	inline bool operator==(const allocatorx_<t_,m_>&, const allocatorx_<_Other,m_>&) _THROW0()
	{	// test for allocator equality (always true)
		return (true);
	}

	template<typename t_,typename m_,typename _Other> 
	inline bool operator!=(const allocatorx_<t_,m_>&, const allocatorx_<_Other,m_>&) _THROW0()
	{	// test for allocator inequality (always false)
		return (false);
	}

	// CLASS allocator<void>
	template<typename m_> class  allocatorx_<void,m_>
	{	// generic allocator for type void
	public:
		typedef void _Ty;
		typedef _Ty  *pointer;
		typedef const _Ty  *const_pointer;
		typedef _Ty value_type;

		template<class _Other>
		struct rebind
		{	// convert an allocator<void> to an allocator <_Other>
			typedef allocatorx_<_Other,m_> other;
		};

		allocatorx_() _THROW0()
		{	// construct default allocator (do nothing)
		}

		allocatorx_(const allocatorx_<_Ty,m_>&) _THROW0()
		{	// construct by copying (do nothing)
		}

		template<class _Other,m_>
		allocatorx_(const allocatorx_<_Other,m_>&) _THROW0()
		{	// construct from related allocator (do nothing)
		}

		template<class _Other,m_>
		allocatorx_<_Ty>& operator=(const allocatorx_<_Other,m_>&)
		{	// assign from a related allocator (do nothing)
			return (*this);
		}
	};
}

#endif //ALLOCATOR_LANGUAGE_XHB_H_