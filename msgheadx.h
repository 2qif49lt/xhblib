#ifndef MESSAGEHEADX_NET_XHB_H_
#define MESSAGEHEADX_NET_XHB_H_

typedef struct _msgheadstruct
{
	unsigned short	uscmd;	//命令码 协议id
	unsigned short	usreserve;
	unsigned int	uilen;	//整个消息的长度
}BaseMsgHead,*PBaseMsgHead;

#endif	//MESSAGEHEADX_NET_XHB_H_