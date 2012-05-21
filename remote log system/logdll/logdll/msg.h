#ifndef MSG_EPOLLX_XHB_H_
#define MSG_EPOLLX_XHB_H_
#pragma   pack(1) 
/*
    app protocal messages command
*/
#define CMD_BEGIN       10000

#define CMD_LOGIN       (CMD_BEGIN + 1) //login 
#define CMD_LOG         (CMD_BEGIN + 2) //log
#define CMD_HEARTBEAT   (CMD_BEGIN + 3) //for keep alive,server return the some cmd and data
#define CMD_END         (CMD_BEGIN + 100)

/*
    app protocal messages format.
*/

//message's public data before all data which to be send.
typedef struct tagmsg_head
{
    unsigned int uilen;
    unsigned int uicmd; //protocal command
}msg_head,*pmsg_head;

//msg_log should be send to server after you connect,and before you send log.
typedef struct tagmsg_login
{
    unsigned int uiver; //the version of msg_login
    unsigned int uiip;  //network ip or local ip
    
    unsigned int uiida; //uiida,b,c,d 
    unsigned int uiidb; //ida means sid now. else nothing.
    unsigned int uiidc;
    unsigned int uiidd;
    
 //   char szspecificity[256];    //whatever you put here will be output in our log system.
                                //this will be a point you can find the log where it begin.
}msg_login,*pmsg_login;

//msg_login have no message return by the server.
//because you will be disconnect if the server refuse you.

//log message. 
typedef struct tagmsg_log
{
    unsigned int uiver; //the version  
    unsigned int uitimet;   //32 bits time_t,no matter which your platorm or application is.
    unsigned int uithreadid;    //which thread output this.
    short slev;   //log level
    unsigned short uslen;   //the date len in the chbuff.if that is a character string,you should count with '\0'
    char chbuff[0];
}msg_log,*pmsg_log;

//msg_log have no message return also..
//because alse..

//server will echo the heartbeat message.
typedef struct tagmsg_heartbeat 
{
    unsigned int uiver; //the version  
    char chbuff[32];    //define any data by youself.
}msg_heartbeat ,*pmsg_heartbeat ;

#pragma   pack() 
//there is no quit message.
#endif //MSG_EPOLLX_XHB_H_
