 #ifndef DEFER_LANGUAGE_XHB_H_
#define DEFER_LANGUAGE_XHB_H_

 /*
  void testdefer()
 {
     FILE* pf = fopen("d:/1.txt","r");
     defer([&](){if(pf)fclose(pf);});
     
     char chbuff[10240] = {};
     if(pf)
     {
         int iread = fread(chbuff,sizeof(char),sizeof(chbuff),pf);
        if(iread > 0)
         cout<<chbuff<<endl;
     }
 }
 */
 #include<functional>
namespace xhb
{
 
 class defer_
 {
 public:
     explicit defer_(std::function<void()> onexit) : m_onexit(onexit),m_bdismiss(false){}
     ~defer_()
     {
         if(!m_bdismiss)
             m_onexit();
     }
     void dismiss(){m_bdismiss = true;}
 private:
     std::function<void()> m_onexit;
     bool m_bdismiss;
 
 private:
     defer_(const defer_& rhs);
     defer_& operator= (const defer_& rhs);
 };
 
#define defer_name_cat(name,line) name##line
#define defer_name(name,line) defer_name_cat(name,line)
#define defer(fn) defer_ defer_name(defer_name_,__LINE__) (fn)

 }
#endif //DEFER_LANGUAGE_XHB_H_