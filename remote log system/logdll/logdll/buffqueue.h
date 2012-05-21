#include "ptrqueuex.h"
using xhb::ptrqueuex;
#include "mthread.h"
#include <new>
#include <time.h>

typedef struct tagbuff_struct 
{
	tagbuff_struct():pbuff(NULL),uilen(0){}
	char*	pbuff;
	unsigned int uilen;
	time_t  tadd;
}Buff_Struct,*PBuff_Struct;

template<int nCapable,bool bReplace,bool benabletimeout>
class buffqueuex
{
public:
	buffqueuex(){}
	~buffqueuex()
	{
		PBuff_Struct pbs = NULL;
		while (pbs = m_queue.pop_front())
		{
			delete [] pbs->pbuff;
			delete pbs;
			pbs = NULL;
		}
	}
public:
	//�ɹ�����0,����ֵΪ������ 1��ʾϵͳ�ڴ治������
	//2��ʾ��������
	int PutData(const char* pbuff,unsigned int uilen)
	{
		mplock lock(&m_lockobject);
		
		if (bReplace == false)
			if (m_queue.isfull())
				return 2;

		PBuff_Struct pbs = new(std::nothrow) Buff_Struct;
		if (pbs == NULL)
		{
			return 1;
		}
		pbs->pbuff = new(std::nothrow) char[uilen];
		if (pbs->pbuff == NULL)
		{
			delete pbs;
			return 1;
		}
		pbs->tadd = time(NULL);
		pbs->uilen = uilen;
		memcpy(pbs->pbuff,pbuff,uilen);
		pbs = m_queue.push_back(pbs);
		if (pbs != NULL)
		{
			ReleaseBuff(pbs->pbuff);
			delete pbs;
		}
		
		return 0;
	}
	//�ɹ�����0,����ֵΪ������
	//1Ϊû������
	//GetData�õ���pbuff ��Ҫreleasebuff�ͷ�
	int GetData(char* &pbuff,unsigned int &uilen)
	{
		mplock lock(&m_lockobject);
		
		PBuff_Struct pbs = NULL;
		while (pbs = m_queue.pop_front())
		{
			if (benabletimeout == true)
			{
				if (time(NULL) - pbs->tadd > 5)	//5�볬ʱ
				{
					ReleaseBuff(pbs->pbuff);
					delete pbs;
					continue;
				}
				else
					break;
			}
			else
				break;
		}
		if (pbs == NULL)
		{
			return 1;
		}
		pbuff = pbs->pbuff;
		uilen = pbs->uilen;
		
		delete pbs;
		return 0;
	}
	
	inline void ReleaseBuff(const char* pbuff){if(pbuff) delete [] pbuff;}
private:
	ptrqueuex<Buff_Struct,nCapable,bReplace> m_queue;
	mlockx_ m_lockobject;
};