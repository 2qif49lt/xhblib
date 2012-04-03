#include <string>
using namespace std;

class mshellstr
{
public:
    mshellstr(){}
    ~mshellstr(){}
public:
    //////////////////////////////////////////////////////////////////////////
    // system 系统设置
    // get_system_install 获取系统安装时间指令
    static void get_system_install(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_system_program 获取Program File下的内容指令
    static void get_system_program(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_system_restart 获取重启指令
    static void get_system_restart(
        string& strwincmd,                            
        string& strlinuxcmd
        );

    // get_system_account 获取所有账户名,如果strname不为空则获取strname的账户信息
    // 不为空则是获取系统里所有账户名指令
    static void get_system_account(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname = ""  //账户名
        );

    // get_system_addaccount 获得添加账户指令
    static void get_system_addaccount(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname       //账户名
        );
    // get_system_delaccount 获得删除账户指令
    static void get_system_delaccount(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname       //账户名
        );
    // get_system_disableaccount 获取禁用某账号指令
    static void get_system_activeaccount(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname,      //账户名
        bool bactive = false        //bactive 默认表示禁用账户
        );   
    // get_system_chgpwd 获取修改账户密码指令
    static void get_system_chgpwd(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname,          //账户名
        const string& strpwd            //账户密码
        );
    // get_system_chgaccount 修改账户名(注意:windows下无此命令)
    static void get_system_chgaccount(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& stroldname,       //老账户名
        const string& strnewname        //新账户名
        );
     
    // get_system_group 获取群组信息的指令
    // 如果strname为空则获取所有的群组
    // 如果strname不为空则获取strname指定的群组信息
    static void get_system_group(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strgroup = "" //组,group
        );

    // get_system_addgroup 添加组的指令
    static void get_system_addgroup(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strgroup      //组,group
        );
    // get_system_delgroup 获取删除组的指令
    static void get_system_delgroup(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strgroup      //组,group
        );
    // get_system_chgusergroup 添加用户名进某组的指令
    static void get_system_addusergroup(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname,      //用户账户名
        const string& strgroup      //组,group
        );
    // get_system_delusergroup 剔除用户在某组的指令
    static void get_system_delusergroup(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname,      //用户账户名
        const string& strgroup      //组,group
        );
    // get_system_adduserpsgroup 添加用户并且加入组
    static void get_system_adduserpsgroup(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname,      //用户账户名
        const string& strpasswd,    //用户密码
        const string& strgroup      //组,group
        );
    // get_system_timesynch 获取同步时间指令
    static void get_system_timesynch(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strtimesrv    //时间服务器
        );
	//
	static void get_system_timesynchbybat(
		string& strwincmd,                            
		string& strlinuxcmd,
		const string& strusrpara    //用户输入的参数
		);

    // get_system_exce 获取执行程序指令  
    static void get_system_exce(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strbinpath    //可执行文件路径
        );
    // get_system_delete 获取删除文件或目录的指令
    static void get_system_deletef(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strpath    //路径(必须要求如果是目录的话,最后面带反斜杠\.)
        );
    // get_system_delshare 获取删除共享目录指令(注意linux无此命令行)
    static void get_system_delshare(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strfolder    //共享目录名     
        );
    // get_system_timezone 获取设置中国时区指令
    static void get_system_timezone(
        string& strwincmd,                            
        string& strlinuxcmd
     /*   const string& strtz = "Shanghai"    //时区,默认是亚洲-上海     */
        );
    // get_system_stopnetbios 获取停止netbios on tcp/ip服务指令
    static void get_system_stopnetbios( 
        string& strwincmd,                         
        string& strlinuxcmd
        );
    // get_system_sysmonlog 获取开始/停止windows性能日志命令
    static void get_system_sysmonlog(
        string& strwincmd,                            
        string& strlinuxcmd,
		bool benable
        );

    // get_system_shutdown 获取关机指令
    static void get_system_shutdown(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    //////////////////////////////////////////////////////////////////////////
    // security 安全设置
    // get_sec_ipsec  获取启动或停止ipsec/iptable的指令
    static void get_sec_ipsec(
        string& strwincmd,                            
        string& strlinuxcmd,
        bool benable        //true启动,false停止
        );
    //////////////////////////////////////////////////////////////////////////
    // system info 系统查看
    static void get_sysinfo_process(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strname       //进程关键字
        );
    // get_sysinfo_restarttime 获取系统重启时间
    static void get_sysinfo_restarttime(
        string& strwincmd,                            
        string& strlinuxcmd
        );

    // 见system里的get_system_account 获取系统用户名指令

    // get_sysinfo_runitem 获取查看系统启动项指令
    static void get_sysinfo_runitem(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_sysinfo_time 获取系统时间指令
    static void get_sysinfo_time(
        string& strwincmd,                            
        string& strlinuxcmd
        );
   // get_sysinfo_dir 获取检查目录指令
    static void get_sysinfo_dir(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strpath,
        const string& strkey = ".exe"
        );
    // get_sysinfo_attrib 获取查询文件信息指令
    static void get_sysinfo_attrib(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strpath
        );
    // get_sysinfo_timezone 获取查询时区设置指令
    static void get_sysinfo_timezone(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_sysinfo_hardsoftinfo 查看服务器软硬件信息
    static void get_sysinfo_hardsoftinfo(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_sysinfo_ietypedurl 获取ie上网记录(地址栏记录)
    static void get_sysinfo_ietypedurl(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    //////////////////////////////////////////////////////////////////////////
    // net
    // get_net_ping 获取ping 命令
    static void get_net_ping(
        string& strwincmd,                              //out, windows下的
        string& strlinuxcmd,                            //out, linux下的
        const string& strdistinip,          //in, 要ping的服务器
        unsigned int uicount = 5);          //in, ping的次数
    // get_net_tracert 获取traceroute命令
    static void get_net_tracert(
        string& strwincmd,                            
        string& strlinuxcmd,  
        const string& strdistin,            //in,目的地址,可以是地址或域名
        unsigned int uimaxhops = 15         //in,最大跳跃次数
        );
    // get_net_ipconfig 获取网卡信息命令
    static void get_net_ipconfig(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_net_link get_net_link 获取检测当前网络状况TCP当前连接 TCP当前监听 UDP当前监听
    static void get_net_link(
        string& strwincmd,                            
        string& strlinuxcmd,
		int nIndex
        );
    // get_net_route 获取路由表信息命令
    static void get_net_route(
        string& strwincmd,                            
        string& strlinuxcmd
        );
    // get_net_urlip 获取查询域名或ip指令
    static void get_net_urlip(
        string& strwincmd,                            
        string& strlinuxcmd,
        const string& strurlip      //域名或者ip
        );
    // get_net_arp 获取arp信息,通过ARP检查IP和MAC命令
    static void get_net_arp(
        string& strwincmd,                            
        string& strlinuxcmd
        );

	// get_net_checktcpport 获取检查某个ip的tcp端口是否可以连接的指令
	static void get_net_checktcpport(
		string& strwincmd,                            
		string& strlinuxcmd,
		const string& strip,
		unsigned short usport
		);

	// get_system_restartinfobydays 根据iday参数获取系统重启记录
	static void get_system_restartinfobydays(
		string& strwincmd,                            
		string& strlinuxcmd,
		int iday	//iday 表示多少天内
		);

	// get_system_ipsecinfo 获取ipsec服务运行状态
	static void get_system_ipsecinfo(
		string& strwincmd,                            
		string& strlinuxcmd
		);
	
	// get_sec_initipsec 获取初始化ipsec命令
	static void get_sec_initipsec(
		string& strwincmd,                            
		string& strlinuxcmd
		);

private:
};