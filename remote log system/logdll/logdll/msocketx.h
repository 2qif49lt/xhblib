#ifndef MSOCKETX_H_
#define MSOCKETX_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#pragma comment(lib,"ws2_32.lib")
#include "winsock2.h"
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //for ntohs
#include <sys/time.h> //for timeval
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
typedef int SOCKET;
#define SOCKET_ERROR  (-1)
#define INVALID_SOCKET (-1)
#endif

#define CAN_READX           1
#define CAN_WRITEX          2
#define CAN_CONNECTX        4
#define CAN_ACCEPTX         8

class msocketx
{
public:
    msocketx();
    virtual ~msocketx();
public:
    // startup ���ȵ���,linux���Բ��õ���
    static bool startup();

    // clearup �������ʱ����,linux���Բ��õ���
    static bool clearup();

    // getlasterror ��ȡ��һ�δ������
    static int getlasterror();

public:
    // attch ����һ��socket
    void attach(SOCKET sock);

    // isvalid �Ƿ���Ч
    inline bool isvalid();

    // close �����ж�isvalid,Ȼ��shutdown,Ȼ��closesocket,���ֵINVALIDSOCKET
    void close();
    
    // setsock �����setsockbuff,setsocktime.
    bool setsock();

    // create ����socket
    bool create(int itype,int iprotocol);

    // bind ��ָ��ip�Ͷ˿�,���pstripΪ���������ip
    bool bind(unsigned short usport,const char* pstrip = NULL);

    // sendto �����վ�
    int sendto(const void* pbuf, unsigned int ilen, unsigned int uiip, unsigned short usport, int iflags);

    // sendto �����վ�
    int sendto(const void* pbuf, unsigned int ilen, const char* pszip, unsigned short usport, int iflags);

    // recvfrom ��������
    int recvfrom(void* pbuf, unsigned int ilen, unsigned int& uiip, unsigned short& usport, int iflags);

    // send ��������
    int send(const void* pbuf, unsigned int ilen,int iflags);

    // recv ��������
    int recv(void* pbuf,unsigned int ilen,int iflags);

    // listen ����
    bool listen(int ibacklog);

    // accept ����
    bool accept(SOCKET& sock,sockaddr* peeraddr,socklen_t* addrlen);

    bool accept(msocketx& sock,sockaddr* peeraddr,socklen_t* addrlen);

    // connect ����,usportΪ�����ֽ���.
    int connect(const char* pszip,unsigned short usport,bool bblock);

    // wait �ȴ�iflagx�¼�,�¼��������CAN_READX...������-1,0��ʾ��ʱ,
    //      �������ؿɲ�������,��&ȡ���ж�
    int wait(unsigned int uimilli,int iflagx);
public:
    // setblock �����Ƿ�����
    bool setblock(bool bblock = false);

    // setsockbuff ����socketϵͳ��������С
    bool setsockbuff(unsigned int uirecvlen = 4 * 1024 * 1024,
        unsigned int uisendlen = 4 * 1024 * 1024);

    // setsocktime ����send\recv������ʱʱ��.
    bool setsocktime(unsigned int uimillisecond = 500);

    // setsocknagle �Ƿ�����nagle�㷨.
    bool setsocknagle(bool benable = true);

    // shutdown     0 read,1 write,2 both
    int shutdown(int ihow);

    // closesocket  ������Ĺص�socket
    int closesocket();

    // getsockname ��ȡ��socket��ǰ�󶨵�ַ,ulip Ϊ������,usportΪ������
    int getsockname(unsigned int& uiip,unsigned short& usport);
    
private:
    SOCKET m_sock;
};
#endif  //MSOCKETX_H_