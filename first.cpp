#include <windows.h>
#include <unordered_map>
#include <algorithm>
class WndProcWM{
   public:
   virtual  LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)=0;
   };
   class WndProc:public WndProcWM{
       protected:
       std::unordered_map<UINT,std::vector<WndProcWM*> > wms;
       public:
       inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
           if(!wms.count(msg))return DefWindowProc(hwnd,msg,wp,lp);
           UINT lresult;
           std::erase(wms[msg],nullptr);
           for(auto wndproc:wms[msg]){
               lresult=wndproc->Do(hwnd,msg,wp,lp);
           }
           return lresult;
       };
       inline auto &add(UINT msg,WndProcWM *wndProcWM_PTR){
           wms[msg].push_back(wndProcWM_PTR);
           return *this;
       }
       ~WndProc(){
           wms.clear();
       }
   };
   class wndProcCMD:public WndProc{
       public:
       inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
           if(!wms.count(wp))return DefWindowProc(hwnd,msg,wp,lp);
           LRESULT l;
           std::erase(wms[wp],nullptr);
           for(auto &obj:wms.at(wp)){
               l=obj->Do(hwnd,msg,wp,lp);
           }
           return l;
       }
   };