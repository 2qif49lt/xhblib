#include <iostream>
#include <string>
#include <list>
#include <vector>
using namespace std;
#include "commonx.h"
#include "logx.h"
using namespace xhb;

#define  T3 "\t\t\t        "
int main()
{
    printf("\t\t\tabc\n");
    printf(T3"T3abc\n");
    basefunctionx_::initiate();
    string strpath = basefunctionx_::get_program_path();
    strpath += "\\log";
    logx<> log;
   // log.InitLog(strpath.c_str(),"1.log",LOG_LV_PRIVATE);	
    log.InitLog();
    int i = 0;
    while (++i)
    {
        log.log(i%LOG_LV_PRIVATE,_T("第 %08d"),i);
    }
	system("pause");
	return 0;
}