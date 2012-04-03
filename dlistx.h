#ifndef DOUBLELINK_LANGUAGE_XHB_H_
#define DOUBLELINK_LANGUAGE_XHB_H_
/************************************************************************/
/* 
双向链表类(不循环)
使用方法和stl list一样。
dlistx_<int> llst;
for (int i = 0; i != 10000; ++i)
{
llst.push_back(i);
}
for (dlistx_<int>::iterator it = llst.begin(); it != llst.end(); ++it)
{
cout<<*it<<endl;
}
llst.clear();

不同点：
多了个erase接口，可以根据成员的指针进行删除
*/
/************************************************************************/
#include "memoryx.h"
namespace xhb
{
	template<typename t_,
		template<typename n_> class m_ = mm_class_defaultx_>
	class dlistx_
	{
	public:
		typedef t_ value_type;
		typedef value_type *pointer;
		typedef value_type &reference;
		typedef const value_type *const_pointer;
		typedef const value_type &const_reference;
		
		class nodex_;
		typedef nodex_ node_type;
		typedef node_type *nodepointer;
		typedef node_type &nodereference;
		typedef const node_type *const_nodepointer;
		typedef const node_type &const_nodereference;

	private:
		class nodex_
		{
		public:
			class iterator;
			friend class iterator;
			friend class dlistx_;
			inline nodepointer next() {return nextptr;}	
			inline nodepointer prev() {return prevptr;}
			//将nodevalue插入到自己之前
			inline nodepointer insert_front(const_nodepointer pnode)
			{
				nodepointer tmp = const_cast<nodepointer>(pnode);
				if (tmp != 0)
				{			
					prevptr->nextptr = tmp;	
					tmp->prevptr = prevptr;	
					tmp->nextptr = this;	
					prevptr = tmp;				
				}
				return tmp;
			}

			inline nodepointer insert_front(const t_& nodevalue)
			{
				nodepointer pnode = m_<node_type>::alloc();
				pnode->m_value = nodevalue;
				return insert_front(pnode);
			}
			
			inline nodepointer insert_front(const_nodereference node)
			{
				return insert_front(node->m_value);
			}
			//将nodevalue插入到自己之后
			inline nodepointer insert_back(const_nodepointer pnode)
			{
				nodepointer tmp = const_cast<nodepointer>(pnode);
				if (tmp)
				{
					tmp->nextptr = nextptr;
					tmp->prevptr = this;
					nextptr->prevptr = tmp;
					nextptr = tmp;
				}
				return tmp;
			}
			inline nodepointer insert_back(const t_& nodevalue)
			{
				nodepointer pnode = m_<node_type>::alloc();
				pnode->m_value = nodevalue;
				return insert_back(pnode);
			}
			inline nodepointer insert_back(const_nodereference node)
			{
				return insert_back(node->m_value);
			}
			

			//移除自己,在外部释放内存
			inline nodepointer remove()
			{
				prevptr->nextptr = nextptr;
				nextptr->prevptr = prevptr;
				return this;
			}
			inline reference value_refer() { return m_value;}
			inline pointer value_ptr() {return &m_value;}
		private:
			t_ m_value;
			nodepointer nextptr;
			nodepointer prevptr;
		};
	public:
		class iterator
		{
			friend class dlistx_;
		public:
			typedef bidirectional_iterator_tag iterator_category;

			iterator():m_nodeptr(0) {}
			iterator(nodepointer ptr):m_nodeptr(ptr) {}
			iterator(const iterator& rhs):m_nodeptr(rhs.m_nodeptr) {}
			iterator& operator= (const iterator& rhs) {m_nodeptr = rhs.m_nodeptr; return *this;}
			iterator& operator= (const_nodepointer ptr) {m_nodeptr = const_cast<nodepointer>(ptr); return *this;}

			reference operator* () {return m_nodeptr->value_refer();}
			const_reference operator* () const {return m_nodeptr->value_refer();}
			pointer operator-> (){return m_nodeptr->value_ptr();}
			const_pointer operator-> () const {return m_nodeptr->value_ptr();}

			inline bool operator== (const iterator& rhs) {return m_nodeptr == rhs.m_nodeptr;}
			inline bool operator!= (const iterator& rhs) {return m_nodeptr != rhs.m_nodeptr;}
			
			iterator& operator++ (){ m_nodeptr = m_nodeptr->next(); return *this;}
			iterator operator++(int) {iterator it(m_nodeptr); m_nodeptr = m_nodeptr->next(); return it;}
			iterator& operator--() {m_nodeptr = m_nodeptr->prev(); return *this;}
			iterator operator--(int) {iterator it(m_nodeptr); m_nodeptr = m_nodeptr->prev(); return it;}
	
		private:
			nodepointer m_nodeptr;
		};
	public:
		typedef const iterator const_iterator;
		dlistx_()
		{
			nodepointer phead = m_<node_type>::alloc();
			nodepointer pend = m_<node_type>::alloc();
			phead->prevptr = 0;
			phead->nextptr = pend;
			pend->prevptr = phead;
			pend->nextptr = 0;
			m_head = phead;
			m_end = pend;
			m_nsize = 0;
		}
		~dlistx_()
		{
			clear();
			m_<node_type>::free(m_head.m_nodeptr);
			m_<node_type>::free(m_end.m_nodeptr);
		}
		inline bool empty(){ return m_nsize == 0;}
		inline iterator begin(){iterator it(m_head); return ++it;}
		inline iterator end(){return m_end;}
		inline size_t size(){return m_nsize;}
		//在链表最后插入
		iterator push_back(const t_& val)
		{
			++m_nsize;
			return m_end.m_nodeptr->insert_front(val);
		}
		//在链表最前面插入
		iterator push_front(const t_& val)
		{
			++m_nsize;
			return m_head.m_nodeptr->insert_back(val);
		}
		//删除操作都返回下一个迭代器
		iterator erase(iterator iter)
		{
			iterator it = iter;
			++it;
			m_<node_type>::free(iter.m_nodeptr->remove());
			--m_nsize;
			return it;
		}
		//通过对象的指针删除
		iterator earse(pointer valueptr)
		{
			//\对象是在nodex_最前面，所以他的指针也是nodex_的指针
			nodepointer pnode = (nodepointer)valueptr;
			iterator it(pnode);
			++it;
			m_<node_type>::free(pnode->remove());
			--m_nsize;
			return it;
		}
		void clear()
		{
			iterator iter = begin();
			while (iter != end())
				iter = erase(iter);
			m_nsize = 0;
		}
	private:
		iterator m_head; //[beg,end)
		iterator m_end;
		size_t m_nsize;
	};
}
#endif //DOUBLELINK_LANGUAGE_XHB_H_
