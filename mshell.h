/************************************************************************/
/* 
                        测试代码
                        #include <iostream>
                        #include <string>
                        #include <list>
                        #include "mshell.h"
                        using namespace std;
                        int main()
                        {
                        cmdexec_ cmd;
                        string strincmd;
                        while(1)
                        {
                        cout<<"$";
                        getline(cin,strincmd);
                        bool bret = cmd.exec(strincmd);
                        if (cmd.issucc())
                        {
                        cout<<"# 命令:"<<cmd.getcmd()<<endl;
                        cout<<"# 返回值:"<<cmd.getvalue()<<endl;
                        cout<<"# 输出:"<<cmd.getout()<<endl;
                        }
                        else
                        {
                        cout<<"# 失败.超过10秒或者输出大于40K"<<endl;
                        }
                        } 

                        system("pause");
                        return 0;
                        }
*/
/************************************************************************/
#ifdef _WIN32
#include "windows.h"
// #define popen(A,B) _popen((A),(B))
// #define pclose(A) _pclose(A)
#else
#include <sys/types.h>  
#include <unistd.h> 
#include <string.h>
#endif

#include <stdio.h>
#include <new>
#include <iostream>
#include <string>
using namespace std;

class cmdexec_
{
public:
    cmdexec_():m_strcmd(""),m_uiexitvalue(0),m_bexec(false),m_strout(""){}
    bool exec(const string& strcmd)
    {
        m_strcmd = strcmd;
        m_uiexitvalue = 0;
        m_bexec = false;
        m_strout = "";
        m_bexec = docmd();
        if (m_bexec && m_strout.length() == 0)
        {
            m_strout = "done none";
        }
        return m_bexec;
    }

    inline string getcmd()const {return m_strcmd;}
    inline int getvalue() const {return m_uiexitvalue;}
    inline bool issucc()const {return m_bexec;}
    inline string getout()const {return m_strout;}

private:
    bool docmd()
    {
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        
        HANDLE hRead,hWrite;
        if (!CreatePipe(&hRead,&hWrite,&sa,40960)) 
            return false;

        char command[1024] = {0};
        strcpy(command,"cmd.exe /C ");
        strcat(command,m_strcmd.c_str());

        char path[1024];
        memset(path, 0,sizeof(path));
        GetSystemDirectory(path,sizeof(path));

        STARTUPINFO si;
        PROCESS_INFORMATION pi; 

        ZeroMemory(&si,sizeof(si));
        ZeroMemory(&pi,sizeof(pi));

        si.cb = sizeof(STARTUPINFO);
  //      GetStartupInfo(&si); 
        si.hStdError = hWrite;            
        si.hStdOutput = hWrite;          
        si.wShowWindow = SW_HIDE;
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

        if(!CreateProcess(NULL,command ,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,path,&si,&pi)) 
        {
            CloseHandle(hWrite);
            CloseHandle(hRead);
            return false;
        }
        int uiwait = WaitForSingleObject(pi.hProcess,10*1000);

        CloseHandle(hWrite);

        //超时,则杀死.
        if (uiwait == WAIT_TIMEOUT) 
        {
            int uitermret = TerminateProcess(pi.hProcess,1234);
            m_uiexitvalue = 1234;

            if (uitermret == 0)
            {
                cout<<"TerminateProcess error: "<<GetLastError()<<endl;
            }

            CloseHandle(pi.hProcess);   
            CloseHandle(pi.hThread); 

            CloseHandle(hRead); 
            return false;
        }

        char* pbuffer = new(std::nothrow) char[40960]; 
        if (pbuffer == NULL)
        {
            CloseHandle(hRead); 
            CloseHandle(pi.hProcess);   
            CloseHandle(pi.hThread);  
            return false;
        }
        memset(pbuffer,0,40960);
        DWORD bytesRead = 0; 
        while (bytesRead < 40960 - 1) 
        {
            DWORD perread = 0;
            if (ReadFile(hRead,pbuffer + bytesRead,40960 - 1 - bytesRead,&perread,NULL) == TRUE)
            {
                if (perread == 0)
                    break;
            }
            else
                break;
            bytesRead += perread;
        }
        CloseHandle(hRead); 
        m_strout = pbuffer;

        DWORD dwexitword = 0;
        GetExitCodeProcess(pi.hProcess,&dwexitword);
        m_uiexitvalue = dwexitword;

        CloseHandle(pi.hProcess);   
        CloseHandle(pi.hThread);   
       
        return true;

#else
        FILE* rs = NULL;  
        char* pbuf = new(nothrow) char[40960]; 
        if (pbuf == NULL)
        {
            return false;
        }
        memset(pbuf,0,40960);
        
        string strtmpcmd = m_strcmd + " 2>&1";
        rs = popen( strtmpcmd.c_str(), "r" ); 
        if (rs == NULL)
        {
            if (pbuf)
            {
                delete [] pbuf;
                pbuf = NULL;
            }
            return false;
        }

        /*unsigned int nret = */
        fread(pbuf, sizeof(char), 40960 - 1,  rs);
        pclose( rs );  
        m_strout = pbuf;
        delete [] pbuf;
        pbuf = NULL;
        return true;
#endif
    }
private:
    string          m_strcmd;       //cmdline
    int        m_uiexitvalue;  //新进程返回值
    bool            m_bexec;        //是否成功创建进程
    string          m_strout;       //输出
};