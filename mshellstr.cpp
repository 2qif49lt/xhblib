#include "mshellstr.h"
#include <sstream>
using namespace std;
void mshellstr::get_net_ping(string& strwincmd, string& strlinuxcmd, const string& strdistinip, unsigned int uicount /* = 5 */)
{
    stringstream ss;
    
    ss<<"ping "<<strdistinip<<" -n " <<uicount;
    strwincmd = ss.str();

    ss.clear();ss.str("");

    ss<<"ping "<<strdistinip<<" -c "<<uicount;
    strlinuxcmd = ss.str();
}
void mshellstr::get_net_tracert( string& strwincmd, string& strlinuxcmd, const string& strdistin, unsigned int uimaxhops /* = 15 */ )
{
    stringstream ss;

    ss<<"tracert "<<strdistin<<" -d -h "<<uimaxhops;
    strwincmd = ss.str();

    ss.clear();ss.str("");
    
    ss<<"traceroute "<<strdistin<<" -n -m "<<uimaxhops;
    strlinuxcmd = ss.str();
}
void mshellstr::get_net_ipconfig( string& strwincmd, string& strlinuxcmd )
{
   strwincmd = "ipconfig";
   
   strlinuxcmd = "ifconfig";
}

void mshellstr::get_net_link( string& strwincmd, string& strlinuxcmd ,int nIndex)
{
	if (nIndex == 0)
	{
		strwincmd = "netstat -ano | findstr \"ESTABLISHED\"";
	}
	else if (nIndex == 1)
	{
		strwincmd = "netstat -ano | findstr \"LISTENING *:*\"";
	}
	else if (nIndex == 2)
	{
		strwincmd = "netstat -ano | findstr \"LISTENING ESTABLISHED *:*\"";
	}

	strlinuxcmd = "netstat -ltuap";	//linux上等待区分
}
void mshellstr::get_net_route( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "netstat -r";
    strlinuxcmd = "netstat -r";
}
void mshellstr::get_net_urlip( string& strwincmd, string& strlinuxcmd,const string& strurlip )
{
    stringstream ss;
    
    ss<<"nslookup "<<strurlip;
    strwincmd = ss.str();
    strlinuxcmd = strwincmd;
}
void mshellstr::get_net_arp( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "arp -a";
    strlinuxcmd = "arp -a";
}
void mshellstr::get_system_install( string& strwincmd, string& strlinuxcmd )
{
    //chcp 936 中文
  //  strwincmd = "chcp 437 && systeminfo | findstr /i /C \"初始安装日期 Original Install Date\"";
    strwincmd = "systeminfo | findstr /i /C \"初始安装日期 Original\"";
    strlinuxcmd = "stat /lost+found/";
}
void mshellstr::get_system_program( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "dir \"%%SystemDrive%%\\Program Files\"";
    strlinuxcmd = "ls -l"; //linux下没有,所以这里弄了个随意的,避免万一的情况下出错
}
void mshellstr::get_system_restart( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "shutdown /r /t 0 /d p:0:0 /c \"restart by agent\"";
    /************************************************************************/
    /* 
    windows 事件查看里会有如下的记录:
    进程 winlogon.exe 已因下列原因为用户 7FGAME-\xuhaibin 开始计算机 7FGAME- 的 重新启动: 其他(计划的)
    原因代码: 0x85000000
    关机类型: 重新启动
    注释: restart by agent
    */
    /************************************************************************/
    strlinuxcmd = "shutdown -r now";
}
void mshellstr::get_system_account( string& strwincmd, string& strlinuxcmd, const string& strname /* = "" */ )
{
    stringstream ss;
    
    ss<<"net user "<<strname;
    strwincmd = ss.str();

    ss.clear();ss.str("");
    ss<<"cat /etc/passwd |grep \""<<strname<<"\"";
    strlinuxcmd = ss.str();
}

void mshellstr::get_system_addaccount( string& strwincmd, string& strlinuxcmd, const string& strname)
{
    stringstream ss;

    ss<<"net user "<<strname<<" /add";
    strwincmd = ss.str();

    ss.clear();ss.str("");

    ss<<"useradd "<<strname;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_delaccount( string& strwincmd, string& strlinuxcmd, const string& strname )
{
    stringstream ss;

    ss<<"net user "<<strname<<" /del";
    strwincmd = ss.str();

    ss.clear();ss.str("");

    ss<<"userdel "<<strname;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_activeaccount( string& strwincmd, string& strlinuxcmd, const string& strname, bool bactive /* = false */ )
{
    stringstream ss;
    
    ss<<"net user "<<strname<<" /active:"<<bactive?("yes"):("no");
    strwincmd = ss.str();

    ss.clear();ss.str("");
    ss<<"passwd "<<strname<<bactive?(" -l"):(" -u");
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_chgpwd( string& strwincmd, string& strlinuxcmd, const string& strname, const string& strpwd )
{
    stringstream ss;
    
    ss<<"net user "<<strname<<" "<<strpwd;
    strwincmd = ss.str();

    ss.clear();ss.str("");
    //echo 'xhb@7fgame!@#' | passwd --stdin xhb
    ss<<"echo '"<<strpwd<<"' | passwd --stdin "<<strname;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_chgaccount( string& strwincmd, string& strlinuxcmd, const string& stroldname, const string& strnewname )
{	
    stringstream ss;
	ss<<"wmic useraccount where name='"<<stroldname<<"' call rename "<<strnewname;
    strwincmd = ss.str();

	 ss.clear();ss.str("");
    ss<<"usermod -l '"<<strnewname<<"' '"<<stroldname<<"'"; 
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_group( string& strwincmd, string& strlinuxcmd, const string& strgroup /* = "" */ )
{
    stringstream ss;
    
    ss<<"net localgroup "<<strgroup;
    strwincmd = ss.str();

    ss.clear();ss.str("");
    ss<<"cat /etc/group | grep \""<<strgroup<<"\"";
    strlinuxcmd = ss.str();

}
void mshellstr::get_system_addgroup( string& strwincmd, string& strlinuxcmd, const string& strgroup )
{
    stringstream ss;

    ss<<"net localgroup "<<strgroup<<" /add";
    strwincmd = ss.str();

    ss.clear();ss.str("");
    ss<<"gourpadd "<<strgroup;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_delgroup( string& strwincmd, string& strlinuxcmd, const string& strgroup)
{
    stringstream ss;

    ss<<"net localgroup "<<strgroup<<" /delete";
    strwincmd = ss.str();

    ss.clear();ss.str("");
    ss<<"gourpdel "<<strgroup;
    strlinuxcmd = ss.str();
}

void mshellstr::get_system_addusergroup( string& strwincmd, string& strlinuxcmd, const string& strname, const string& strgroup )
{
    stringstream ss;
    
    ss<<"net localgroup "<<strgroup<<" "<<strname<<" /add";
    strwincmd = ss.str();

    ss.clear();ss.str("");
    //usermod -G root xhb
    ss<<"usermod -G "<<strgroup<<" "<<strname;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_delusergroup( string& strwincmd, string& strlinuxcmd, const string& strname, const string& strgroup )
{
    stringstream ss;
        
    ss<<"net localgroup "<<strgroup<<" "<<strname<<" /delete";
    strwincmd = ss.str();

     ss.clear();ss.str("");
     ss<<"gpasswd -d "<<strname<<" "<<strgroup;
     strlinuxcmd = ss.str();
}
void mshellstr::get_system_adduserpsgroup( string& strwincmd, string& strlinuxcmd, const string& strname, const string& strpasswd, const string& strgroup )
{
    stringstream ss;
    
// C:\>net user 123456 /add && net user 123456 123456 && net localgroup administrat
//       ors 123456 /add
    ss<<"net user "<<strname<<" /add && net user "<<strname<<" "<<strpasswd<<" && net localgroup "<<strgroup<<" "<<strname;
    strwincmd = ss.str();
//     [root@xuhaibin etc]# useradd xxxx1 && echo '7fgame@7fgame' | passwd xxxx1 --stdin && usermod -G root xxxx1
//         更改用户 xxxx1 的密码 。
//         passwd： 所有的身份验证令牌已经成功更新。
    ss.clear();ss.str("");
    ss<<"useradd "<<strname<<" 2>&1 && echo '"<<strpasswd<<"' | passwd "<<strname<<" --stdin 2>&1 && usermod -G "<<strgroup<<" "<<strname;
    strlinuxcmd = ss.str();
}   
void mshellstr::get_system_timesynch( string& strwincmd, string& strlinuxcmd, const string& strtimesrv)
{
    stringstream ss;

// C:\Documents and Settings\7fgame>net time \\10.0.60.32 /set /yes
//       \\10.0.60.32 的当前时间是 2011/12/27 下午 08:29
// 
//       命令成功完成。
    //\\192.168.1.2\ipc$ "" /user:guest 建立空连接
    ss<<"net use "<<strtimesrv<<"\\ipc$ \"\" /user:guest && net time "<<strtimesrv<<" /set /yes";
    strwincmd = ss.str();

    ss.clear();ss.str("");
//     [root@xuhaibin ~]# ntpdate clock.redhat.com
//     27 Dec 20:23:19 ntpdate[3600]: step time server 66.187.233.4 offset 8.958189 sec
    ss<<"ntpdate "<<strtimesrv;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_timesynchbybat( string& strwincmd, string& strlinuxcmd, const string& strusrpara )
{
	stringstream ss;
	ss<<"winntp.bat "<<strusrpara;

	strwincmd = ss.str();
	strlinuxcmd = "echo can not do it";
}
void mshellstr::get_system_exce( string& strwincmd, string& strlinuxcmd, const string& strbinpath )
{
    strwincmd = strbinpath;
    strlinuxcmd = strbinpath;
}
void mshellstr::get_system_deletef( string& strwincmd, string& strlinuxcmd, const string& strpath )
{
//      rd /s /q d:\folder\ 删除目录
//      del /F /Q e:\1.txt  删除文件
    char chlast = strpath[strpath.length() - 1 ];
    bool bfolder = false;
    if (chlast == '\\' || chlast == '/')
    {
        bfolder = true;
    }

    stringstream ss;
    if (bfolder)
        ss<<"rd /s /q "<<strpath;
    else
        ss<<"del /F /Q "<<strpath;
    strwincmd = ss.str();

    ss.clear();ss.str("");
//      rm -rf /var/log/httpd/access/
//      rm -f /var/log/httpd/access.log
    if (bfolder)
        ss<<"rm -rf "<<strpath;
    else
        ss<<"rm -f "<<strpath;
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_delshare( string& strwincmd, string& strlinuxcmd, const string& strfolder )
{
    stringstream ss;

    ss<<"net share "<<strfolder<<" /delete";
    strwincmd = ss.str();

    ss.clear();ss.str("");
    strlinuxcmd = "echo can not do it";
}
void mshellstr::get_system_timezone( string& strwincmd, string& strlinuxcmd/*, const string& strtz / * = "Shanghai" * /*/ )
{
    stringstream ss;
    strwincmd = "echo tzutil is available after win7";
    ss.clear();ss.str("");
    //cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && hwclock
    ss<<"cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime 2>&1 && hwclock";
    strlinuxcmd = ss.str();
}
void mshellstr::get_system_stopnetbios( string& strwincmd, string& strlinuxcmd)
{
    //agent的小tool程序,需要将程序路径加入环境变量
    strwincmd = "netbios.exe";
    strlinuxcmd = "echo can not do it";
}
void mshellstr::get_system_sysmonlog( string& strwincmd, string& strlinuxcmd,bool benable)
{
	if (benable)
	{
		strwincmd = "sc config SysmonLog start= auto && sc start SysmonLog";
		strlinuxcmd = "echo can not do it";
	}
	else
	{
		strwincmd = "sc config SysmonLog start= disabled && sc stop SysmonLog";
		strlinuxcmd = "echo can not do it";
	}
}
void mshellstr::get_system_shutdown( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "shutdown /s /t 0 /d p:0:0 /c \"shutdown by agent\"";
    strlinuxcmd = "shutdown -h now";
}
void mshellstr::get_sec_ipsec( string& strwincmd, string& strlinuxcmd, bool benable )
{
    if (benable)
    {
        strwincmd = "net start PolicyAgent";
        strlinuxcmd = "service iptables start";
    }
    else
    {
        strwincmd = "net stop PolicyAgent";
        strlinuxcmd = "service iptables stop";
    }
}
void mshellstr::get_sysinfo_process( string& strwincmd, string& strlinuxcmd, const string& strname )
{
    stringstream ss;
    if (strname == "")
    {
		strwincmd = "tasklist";
		strlinuxcmd = "ps -aux";
    }
	else
	{
		ss<<"tasklist | findstr -i \""<<strname<<"\"";
		strwincmd = ss.str();

		ss.clear();ss.str("");
		ss<<"ps -aux | grep "<<strname;
		strlinuxcmd = ss.str();
	}
}
void mshellstr::get_sysinfo_restarttime( string& strwincmd, string& strlinuxcmd )
{
 //   strwincmd = "chcp 437 && systeminfo | findstr /i \"系统启动时间  time\"";
    strwincmd = "systeminfo | findstr /i \"系统启动时间 time\"";
    strlinuxcmd = "uptime";
}
void mshellstr::get_sysinfo_runitem( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "reg query HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    strlinuxcmd = "chkconfig --list";
}
void mshellstr::get_sysinfo_time( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "date /t && time /t";
    strlinuxcmd = "date";
}
void mshellstr::get_sysinfo_dir( string& strwincmd, string& strlinuxcmd, const string& strpath,const string& strkey )
{
    stringstream ss;
    
    bool bkey = strkey.length() != 0;
    if (bkey)
    {
         ss<<"dir "<<strpath<<" | findstr -i \""<<strkey<<"\"";
         strwincmd = ss.str();

         ss.clear();ss.str("");
         ss<<"ls -l "<<strpath<<" | grep "<<strkey;
         strlinuxcmd = ss.str();
    }
    else
    {
        ss<<"dir "<<strpath;
        strwincmd = ss.str();

        ss.clear();ss.str("");
        ss<<"ls -l "<<strpath;
        strlinuxcmd = ss.str();
    }
}
void mshellstr::get_sysinfo_attrib( string& strwincmd, string& strlinuxcmd, const string& strpath )
{
    stringstream ss;
    //>dir d:\server.ini & attrib d:\server.ini
    ss<<"dir "<<strpath<<" & attrib "<<strpath;
    strwincmd = ss.str();

    //ls -l /root/hello.c
    ss.clear();ss.str("");
    ss<<"ls -l "<<strpath;
    strlinuxcmd = ss.str();
}
void mshellstr::get_sysinfo_timezone( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "w32tm /tz";
    strlinuxcmd = "date -R";
}
void mshellstr::get_sysinfo_hardsoftinfo( string& strwincmd, string& strlinuxcmd )
{
    strwincmd = "wininfo.exe";
    strlinuxcmd = "head -n 1 /etc/issue ; uptime ; top -n 1 |grep Cpu ; free -m ;df -h";
}
void mshellstr::get_sysinfo_ietypedurl( string& strwincmd, string& strlinuxcmd )
{
   // strwincmd = "reg query \"HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedUrls\"";
	strwincmd = "IeHistory.exe";
    strlinuxcmd = "echo can not do it";
}
void mshellstr::get_net_checktcpport( string& strwincmd, string& strlinuxcmd, const string& strip, unsigned short usport )
{
	stringstream ss;
	ss<<"checktcp.exe "<<strip<<' '<<usport;
	strwincmd = ss.str();
	strlinuxcmd = strwincmd;
}
void mshellstr::get_system_restartinfobydays( string& strwincmd, string& strlinuxcmd, int iday )
{
	stringstream ss;
	ss<<"RestartLog.exe "<<iday;
	strwincmd = ss.str();

	strlinuxcmd = "echo can not do it";
}
void mshellstr::get_system_ipsecinfo( string& strwincmd, string& strlinuxcmd )
{
	strwincmd = "sc queryex policyagent";
	strlinuxcmd = "service iptables status";
}
void mshellstr::get_sec_initipsec( string& strwincmd, string& strlinuxcmd )
{
	strwincmd = "netsh ipsec static delete policy all";
	strlinuxcmd = "echo can not do it";
}
