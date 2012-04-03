#ifndef MEMORY_SYSTEM_XHB_H_
#define MEMORY_SYSTEM_XHB_H_
/************************************************************************/
/* 
#include "lockx.h"
#include "memoryx.h"
using xhb::bitpoolx_;
using xhb::criticalsectionx_;
 bitpoolx_<> xhbpool;
 或者
bitpoolx_<criticalsectionx_> xhbpool;


测试用例：
const DWORD dwCount = 1000000;
size_t dwSize = 100;

bitpoolx_<>  xhbpool;
vector<char*> arr(dwCount);
DWORD dwLast = 0;

while (1)
{
{
CAutoTimer a("xhb Pool");
for(DWORD i = 0; i != dwCount; ++i)
{
arr[i] = xhbpool.alloc(dwSize);
memset(arr[i],i,dwSize);
if( i % 2 == 0 )
xhbpool.free(arr[i]);
}

for(DWORD i = 0; i != dwCount; ++i)
{
if( i % 2 != 0 )
xhbpool.free(arr[i]);
}
}

{
CAutoTimer a("system new"); 
for(DWORD i = 0; i != dwCount; ++i)
{
arr[i] = new char[dwSize];
memset(arr[i],i,dwSize);
if( i % 2 == 0 )
delete arr[i];
}

for(DWORD i = 0; i != dwCount; ++i)
{
if( i % 2 != 0 )
delete arr[i];
}
}
Sleep(500);
}
不同的数量级和大小对比，速度大概是百到千倍级别。
*/
/************************************************************************/

#if _MSC_VER >= 1500	//编译器版本在2008sp1之后
#include <type_traits>
using std::tr1::has_trivial_destructor;
#else
//如果有boost库则修改路径,并将下面的error去掉。
#error 编译器版本要求2008sp1及以上! ):
#include <boost/type_traits.hpp>
using boost::has_trivial_destructor;
#endif

#include "windows.h"
#include <list>
using std::list;

#include "commonx.h"
#include "lockx.h"
namespace xhb
{

	//////////////////////////////////////////////////////////////////////////
	//默认内存管理存接口,提供其他申请和释放方式需要有这2个接口
	//用来申请对象
	template<typename t_>
	class  mm_class_defaultx_
	{
	public:
		static t_* alloc() {return new(nothrow) t_;}
		static void free(t_* p) {if(p) delete p;}
	};

	//下面的接口类 需要自己改写，默认是new delete
	//用来简单申请内存
	class mm_char_defaultx_
	{
	public:
		inline static void* alloc(size_t ncount){return ::operator new(ncount);}
		inline static void free(void* ptr){::operator delete(ptr);}
		template<typename t_>
		inline static t_* alloc(){return ::new(nothrow) t_;}
		template<typename t_>
		inline static void free(t_* p){if(p) delete p;}
	};

	//////////////////////////////////////////////////////////////////////////
	//粒度内存池
	template<typename l_ =  falselockx_,	//默认是假锁
		unsigned int particle_min = 64,	//最小粒度 64字节,依次以2递增到max 128 256 512 
		unsigned int particle_max = 1024*1024,	//最大粒度 1M如果申请大于1M的内存则失败
		unsigned int alloc_size = 2*1024*1024,	//每次从系统申请的内存块大小,该值在几M到几十M之间最好
		unsigned int capability = (2*1024*1024*1024UL)>	//内存池最大申请能力2G
		class bitpoolx_
		{
		private:
			struct memory_pool_head		//每次从系统申请的内存池
			{
				char*			pbuff;
				unsigned int	nlen;
			};
			struct memory_node_head	//每个内存块的头部
			{
				char* nnextfreeptr;	//下一个空闲内存块的指针
				unsigned int nparticle;	//内存块粒度
			};
		public:
			bitpoolx_():m_size(0)
			{
				m_headsize = sizeof(memory_node_head);
				memset(m_free,0,sizeof(char*) * 32);
				memset(m_count,0,sizeof(m_count));
				memset(m_all,0,sizeof(m_count));
			}
			bitpoolx_(unsigned long ulcapability,unsigned int uipersize)
			{
				m_headsize = sizeof(memory_node_head);
				memset(m_free,0,sizeof(char*) * 32);
				memset(m_count,0,sizeof(m_count));
				memset(m_all,0,sizeof(m_count));
				m_size = 0;
			}

			~bitpoolx_()
			{
				for (list<memory_pool_head>::iterator iter = m_pool.begin(); iter != m_pool.end(); ++iter)
				{
					memory_pool_head &pool = *iter;
					if (pool.pbuff != NULL)
					{
						::VirtualUnlock(pool.pbuff, pool.nlen);
						::VirtualFree(pool.pbuff, pool.nlen, MEM_RELEASE);
					}
				}
			}
			template<typename t_>
			inline t_ *alloc()		//为类分配1个 pool.alloc<t_>();
			{
				return ::new (alloc(sizeof(t_))) t_;
			}

			template<typename t_>
			inline t_ *alloc(size_t n)	//为类分配多个 pool.alloc<t_>(n);
			{
				return ::new (alloc(sizeof(t_) * n + 4)) t_[n];
			}
			char* alloc(size_t nsize)
			{
				if (nsize == 0 || nsize > particle_max)
					return NULL;

				char* pmemory = NULL;
				unsigned int nparticle = getparticle(nsize);

				pmemory = memory_pool_get(nparticle);
				if (pmemory == NULL)
				{
					//如果没有则会扩大内存池
					unsigned int nallosize = memory_pool_new(nparticle,alloc_size);
					if (nallosize == 0)
						return NULL;
					pmemory = memory_pool_get(nparticle);
					if (pmemory == NULL)
						return NULL;
				}
				return pmemory;
			}
			template<typename t_>
			inline void free(t_* p,size_t n)
			{
				if(p == NULL)
					return;
				size_t offset = 0;
				if (has_trivial_destructor<t_>::value == false)
				{
					t_* tmp = p;
					for (size_t i = 0; i != n; ++i,++tmp)
						tmp->~t_();		
					offset = 4;
				}
				memory_pool_free(reinterpret_cast<char*>(p) - offset);
			}
			template<typename t_>
			inline void free(const t_* cp,size_t n)
			{
				if(cp == NULL)
					return;
				size_t offset = 0;
				if (has_trivial_destructor<t_>::value == false)
				{
					const t_* tmp = cp;
					for (size_t i = 0; i != n; ++i,++tmp)
					{
						tmp->~t_();
					}			
					offset = 4;
				}	
				t_* p = const_cast<t_*>(cp);
				memory_pool_free(reinterpret_cast<char*>(p) - offset);
			}

			template<typename t_>
			inline void free(t_* p)
			{
				if(p == NULL)
					return;
				if (has_trivial_destructor<t_>::value == false)
				{
					p->~t_();
				}		
				memory_pool_free(reinterpret_cast<char*>(p));
			}
			template<typename t_>
			inline void free(const t_* cp)
			{
				if(p == NULL)
					return;
				if (has_trivial_destructor<t_>::value == false)
				{
					p->~t_();
				}	
				t_* p = const_cast<t_*>(cp);
				memory_pool_free(reinterpret_cast<char*>(p));
			}
		public:
			inline unsigned long size()
			{
				return m_size;
			}
			inline void count(int (&arr)[32])
			{
				memcpy(arr,m_count,sizeof(arr));
			}
			inline void all(int (&arr)[32])
			{
				memcpy(arr,m_all,sizeof(arr));
			}
			//添加一个适合nsize大小的粒度,大小为ullen的内存池
			//ullen可以比alloc_size大,但不能大于capability
			//调用者自己保证参数合法合适
			bool initl(size_t nsize,unsigned int ullen = alloc_size)			
			{
				unsigned int nparticle = getparticle(nsize);
				if (m_size + ullen > capability)
					return false;
				unsigned int nretlen = memory_pool_new(nparticle,ullen);
				return nretlen;
			}

			//添加一个适合nsize大小的粒度,个数为10000个的内存池,调用者保证2参数相乘不会溢出
			//调用者自己保证参数合法合适
			bool initc(size_t nsize,unsigned int ulcount = 10000)				
			{
				return initl(nsize,nsize * ulcount);
			}
		
		private:
			//get一个空闲的符合nparticle的内存
			inline char* memory_pool_get(size_t nparticle) 
			{
				lockx_<l_> lock(&m_lockobject);

				size_t nindex = getindex(nparticle);
				char* pret = m_free[nindex];
				if (pret)
				{
					memory_node_head* phead = NULL; //需要改变指针指向空闲的节点
					phead = reinterpret_cast<memory_node_head*>(pret - m_headsize);		
					m_free[nindex] = phead->nnextfreeptr;
					--m_count[nindex];
				}
				return pret;
			}

			//从系统申请内存,如果失败返回0,成功返回申请的字节数,调用者自己保证ullen合法
			inline unsigned int memory_pool_new(size_t nparticle,unsigned int ullen = alloc_size)																																		//调用者保证ullen应该是个大于等于
			{	
				lockx_<l_> lock(&m_lockobject);

				unsigned int nblockcount = ullen / nparticle;
				size_t nlen = ullen + m_headsize * nblockcount;

				if (m_size + nlen > capability)
					return 0;

				void *p = ::VirtualAlloc(NULL, nlen, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);
				if(p == NULL)
					return 0;
				::VirtualLock(p, nlen);

				char* plst = (char*)p;		
				m_size += nlen;

				size_t nnodelen = m_headsize + nparticle;

				char* pmemory = plst + m_headsize;
				char* ptmp = NULL;
				memory_node_head* phead = NULL;

				for (size_t t = 0; t != nblockcount;++t)
				{
					ptmp = plst + nnodelen * t;
					phead = reinterpret_cast<memory_node_head*>(ptmp);
					phead->nnextfreeptr = ptmp + nnodelen + m_headsize;
					phead->nparticle = nparticle;
				}
				phead->nnextfreeptr = NULL;

				size_t nindex = getindex(nparticle);
				m_free[nindex] = pmemory;
				m_count[nindex] += nblockcount;
				m_all[nindex] += nblockcount;

				memory_pool_head pool;
				pool.nlen = nlen;
				pool.pbuff = plst;
				m_pool.push_back(pool);

				return nlen;
			}
			inline void memory_pool_free(char* pmemory)
			{
				lockx_<l_> lock(&m_lockobject);
				memory_node_head* phead = reinterpret_cast<memory_node_head*>(pmemory - m_headsize);
				unsigned int nindex = getindex(phead->nparticle);
				phead->nnextfreeptr = m_free[nindex];
				m_free[nindex] = pmemory;	
				++m_count[nindex];
			}
		private:
			//获取大于等于nsize的粒度
			inline unsigned int getparticle(unsigned int nsize) 
			{
				unsigned int x = nsize;
				x |= (x >> 1);
				x |= (x >> 2);
				x |= (x >> 4);
				x |= (x >> 8);
				x |= (x >> 16);    
				x += 1;
				if (x < particle_min)
				{
					x = particle_min;
				}
				return x;
			}
			
			//获取nnum的2的幂,即是粒度对应的数组索引
			//这里用switch case是为了速度原因
			inline unsigned int getindex(unsigned int nparticle)
			{
				unsigned int nret = 0;
				switch (nparticle)
				{
				case 1<<0:
					return 0;
					break;
				case 1<<1:
					return 1;
					break;
				case 1<<2:
					return 2;
					break;
				case 1<<3:
					return 3;
					break;
				case 1<<4:
					return 4;
					break;
				case 1<<5:
					return 5;
					break;
				case 1<<6:
					return 6;
					break;
				case 1<<7:
					return 7;
					break;

				case 1<<8:
					return 8;
					break;
				case 1<<9:
					return 9;
					break;
				case 1<<10:
					return 10;
					break;
				case 1<<11:
					return 11;
					break;
				case 1<<12:
					return 12;
					break;
				case 1<<13:
					return 13;
					break;
				case 1<<14:
					return 14;
					break;
				case 1<<15:
					return 15;
					break;

				case 1<<16:
					return 16;
					break;
				case 1<<17:
					return 17;
					break;
				case 1<<18:
					return 18;
					break;
				case 1<<19:
					return 19;
					break;
				case 1<<20:
					return 20;
					break;
				case 1<<21:
					return 21;
					break;
				case 1<<22:
					return 22;
					break;
				case 1<<23:
					return 23;
					break;

				case 1<<24:
					return 24;
					break;
				case 1<<25:
					return 25;
					break;
				case 1<<26:
					return 26;
					break;
				case 1<<27:
					return 27;
					break;
				case 1<<28:
					return 28;
					break;
				case 1<<29:
					return 29;
					break;
				case 1<<30:
					return 30;
					break;
				case 1<<31:
					return 31;
					break;
				default:
					break;
				}
				return nret;
			}
			inline bool ispower(unsigned int nnum)
			{
				return (nnum & (nnum - 1)) == 0;
			}
		private:	
			l_ m_lockobject;	//锁
			list<memory_pool_head> m_pool;	//所有从系统申请的内存
			unsigned int		m_size;				//内存大小
			unsigned int		m_headsize;			//memory_node_head的size;
			//每个元素为对应索引的幂的粒度的链表头结点指针.
			//如:m_free[6]对应的内存粒度是2^6=64,m_free[0]对应的是2^0=1;m_free[31]对应的是2^31
			//但是不存在那么多,默认只会64,128,256,512,1024,2048.....1M的粒度
			char* m_free[32];			
			int	m_count[32];	//粒度对应的空闲的个数
			int m_all[32];		//粒度对应的总共的个数
		};
}
#endif //MEMORY_SYSTEM_XHB_H_