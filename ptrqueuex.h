#ifndef PTRQUEUEX_LANGUAGE_XHB_H_
#define PTRQUEUEX_LANGUAGE_XHB_H_

//简单静态数组指针队列.
//该队列内部保存的是T类型的指针.

namespace xhb
{
	//bReplace表示如果队列满了,是否继续插如队尾并将老元素顶出
	//nCapable表示多大的队列
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
		//加入队列尾,返回被顶出的元素指针.没满则不会顶出,返回NULL
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
		//获取第1个元素,不会删除队列里的
		inline _TPtr front()
		{
			if (m_nSize == 0)
				return NULL;
			return m_arr[m_nBeg];
		}
		//取走第1个元素,并且会删除队列里的
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
		int			m_nCapable;		//限制
		int			m_nSize;		//当前元素数目
		int			m_nBeg;		//首索引
		int			m_nEnd;		//end索引,[nBeg,nEnd),nBeg == nEnd 时，可能是没数据或满数据.取决于nSize;
		_TPtr		m_arr[nCapable];
	};
}
#endif //PTRQUEUEX_LANGUAGE_XHB_H_