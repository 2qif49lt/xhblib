#ifndef PTRQUEUEX_LANGUAGE_XHB_H_
#define PTRQUEUEX_LANGUAGE_XHB_H_

namespace xhb
{
	template<typename T,int nCapable ,bool bReplace = true>
	class ptrqueuex
	{
	public:
		typedef T* _TPtr;
		typedef ptrqueuex<T,nCapable,bReplace> _Myt;
		ptrqueuex()
		{
			m_nCapable = nCapable;
			m_nBeg = 0;
			m_nEnd = 0;
			m_nSize = 0;
		}
		~ptrqueuex(){}
		ptrqueuex(const _Myt& rhs)
		{
			m_nCapable = nCapable;
			m_nSize = rhs.m_nSize;
			m_nBeg = rhs.m_nBeg;
			m_nEnd = rhs.m_nEnd;
			memcpy(m_arr,rhs.m_arr,sizeof(m_arr));
		}
		_Myt & operator= (const _Myt& rhs)
		{
			if (this != &rhs)
			{
				m_nSize = rhs.m_nSize;
				m_nBeg = rhs.m_nBeg;
				m_nEnd = rhs.m_nEnd;
				memcpy(m_arr,rhs.m_arr,sizeof(m_arr) );
			}	
			return *this;
		}
		inline _TPtr push_back(_TPtr ptr)
		{
			if (m_nCapable == m_nSize)		//满了
			{
				if (bReplace == false)
					return NULL;
				_TPtr t = m_arr[m_nBeg];
				m_arr[m_nBeg ] = ptr;
				m_nBeg++;
				m_nBeg %= nCapable;
				m_nEnd = m_nBeg;
				return t;
			}
			else
			{
				++m_nSize;
				m_arr[m_nEnd] = ptr;
				m_nEnd++;
				m_nEnd %= nCapable;
				return NULL;
			}
		}
		inline _TPtr front()
		{
			if (m_nSize == 0)
				return NULL;
			return m_arr[m_nBeg];
		}
		inline _TPtr pop_front()
		{
			if (m_nSize == 0)
				return NULL;
			_TPtr t = m_arr[m_nBeg];
			m_nBeg++;
			m_nBeg %= nCapable;
			m_nSize--;
			return t;
		}
		inline int size(){return m_nSize;}
		inline int capable() { return nCapable;}
		inline bool isfull(){ return m_nSize == nCapable;}
	private:
		int			m_nCapable;	
		int			m_nSize;
		int			m_nBeg;
		int			m_nEnd;
		_TPtr		m_arr[nCapable];
	};
}
#endif //PTRQUEUEX_LANGUAGE_XHB_H_