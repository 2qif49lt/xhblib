#ifndef MEMORY_SYSTEM_XHB_H_
#define MEMORY_SYSTEM_XHB_H_
#include <list> 
using namespace std;
#include "mthread.h"
namespace xhb
{
	template<typename l_ =  falselockx_,
		unsigned int particle_min = 64,
		unsigned int particle_max = 1024*1024,
		unsigned int alloc_size = 2*1024*1024,
		unsigned int capability = (2*1024*1024*1024UL)>
		class bitpoolx_
		{
		private:
			struct memory_pool_head
			{
				char* pbuff;
				unsigned int nlen;
			};
			struct memory_node_head
			{
				char* nnextfreeptr;
				unsigned int nparticle;
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
				for (typename list<memory_pool_head>::iterator iter = m_pool.begin(); iter != m_pool.end(); ++iter)
				{
					memory_pool_head &pool = *iter;
					if (pool.pbuff != NULL)
					{
						delete [] pool.pbuff;
					}
				}
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
					unsigned int nallosize = memory_pool_new(nparticle,alloc_size);
					if (nallosize == 0)
						return NULL;
					pmemory = memory_pool_get(nparticle);
					if (pmemory == NULL)
						return NULL;
				}
				return pmemory;
			}
            inline void free(const void* p)
            {
                if(p == NULL)
                    return;
                void* tp = const_cast<void*>(p);
                memory_pool_free(reinterpret_cast<char*>(p));
            }
            inline void free(void* p)
            {
                if(p == NULL)
                    return;
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

			bool initl(size_t nsize,unsigned int ullen = alloc_size)
			{
				unsigned int nparticle = getparticle(nsize);
				if (m_size + ullen > capability)
					return false;
				unsigned int nretlen = memory_pool_new(nparticle,ullen);
				return nretlen;
			}

			bool initc(size_t nsize,unsigned int ulcount = 10000)
			{
				return initl(nsize,nsize * ulcount);
			}

		private:

			inline char* memory_pool_get(size_t nparticle)
			{
				lockx_<l_> lock(&m_lockobject);

				size_t nindex = getindex(nparticle);
				char* pret = m_free[nindex];
				if (pret)
				{
					memory_node_head* phead = NULL;
					phead = reinterpret_cast<memory_node_head*>(pret - m_headsize);
					m_free[nindex] = phead->nnextfreeptr;
					--m_count[nindex];
				}
				return pret;
			}


			inline unsigned int memory_pool_new(size_t nparticle,unsigned int ullen = alloc_size)																																		//调用者保证ullen应该是个大于等于
			{
				lockx_<l_> lock(&m_lockobject);

				unsigned int nblockcount = ullen / nparticle;
				size_t nlen = ullen + m_headsize * nblockcount;

				if (m_size + nlen > capability)
					return 0;

                char *p = new(nothrow) char[nlen];
                if(p == NULL)
                    return 0;

				char* plst = p;
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
			l_ m_lockobject;
			list<memory_pool_head> m_pool;
			unsigned int		m_size;
			unsigned int		m_headsize;
			char* m_free[32];
			int	m_count[32];
			int m_all[32];
		};
}
#endif //MEMORY_SYSTEM_XHB_H_