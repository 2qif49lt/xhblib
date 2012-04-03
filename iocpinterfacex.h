#ifndef IOCPINTERFACEX_NET_XHB_H_
#define IOCPINTERFACEX_NET_XHB_H_
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;
class iocpinterfacex_
{
public:
	iocpinterfacex_(){}
	virtual ~iocpinterfacex_(){}
public:
	//返回0 表示成功,其他表示出错
	//接收到个消息
	virtual int on_recv_msg(void* pdata,unsigned int uilen,DWORD dwip,unsigned short usport,DWORD dwSocketID) = 0;
	//一个客户端连接了
	virtual int on_accepted(DWORD dwip,unsigned short usport,DWORD dwSocketID)=0;
	//一个客户端断开了
	virtual int on_closed(DWORD dwip,unsigned short usport,DWORD dwSocketID)=0;
	//连接上服务器
	virtual int on_connected(DWORD dwip,unsigned short usport,DWORD dwSocketID)=0;
};
class iocphandlex_ :public iocpinterfacex_
{
public:
	virtual int on_recv_msg(void* pdata,unsigned int uilen,DWORD dwip,unsigned short usport,DWORD dwSocketID)
	{
		printf("on_recv_msg %ld %d %ld %ld\n",dwip,usport,dwSocketID,uilen);
		return 0;
	}
	virtual int on_accepted(DWORD dwip,unsigned short usport,DWORD dwSocketID)
	{
		printf("on_accepted %ld %d %ld\n",dwip,usport,dwSocketID);
		return 0;
	}
	virtual int on_closed(DWORD dwip,unsigned short usport,DWORD dwSocketID)
	{
		printf("on_closed %ld %d %ld\n",dwip,usport,dwSocketID);
		return 0;
	}
	virtual int on_connected(DWORD dwip,unsigned short usport,DWORD dwSocketID)
	{
		printf("on_connected %ld %d %ld\n",dwip,usport,dwSocketID);
		return 0;
	}
};
#endif	//IOCPINTERFACEX_NET_XHB_H_