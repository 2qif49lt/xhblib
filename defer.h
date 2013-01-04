 #ifndef DEFER_LANGUAGE_XHB_H_
#define DEFER_LANGUAGE_XHB_H_

 /*
 void testdefer()
 {
     FILE* pf = fopen("d:/1.txt","r");
     defer filehandle([&](){if(pf)fclose(pf);});
     
     char chbuff[10240] = {};
     
     int iread = fread(chbuff,sizeof(char),sizeof(chbuff),pf);
     if(iread > 0)
         cout<<chbuff<<endl;

 }
 */
 #include<functional>
namespace xhb
{
 class defer
 {
 public:
     explicit defer(std::function<void()> onexit) : m_onexit(onexit),m_bdismiss(false){}
     ~defer()
     {
         if(!m_bdismiss)
             m_onexit();
     }
     void dismiss(){m_bdismiss = true;}
 private:
     std::function<void()> m_onexit;
     bool m_bdismiss;
 
 private:
     defer(const defer& rhs);
     defer& operator= (const defer& rhs);
 };

 }
#endif //DEFER_LANGUAGE_XHB_H_