#include <functional>
#include <any>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <unordered_map>
class WndProcWrapper{
    protected:
    std::vector<std::any> share;//initalizer_list
    std::unordered_multimap<UINT,std::function<LRESULT(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)> > wndproc;
    public:
    WndProcWrapper(decltype(share) share):share(share){
        
    }
    template <class T>
    inline auto get(int index) noexcept(std::any_cast){
        return std::any_cast<T>(index);
    }
    template <class T>
    inline auto &set(int index,T value){
        share[index]=value;
        return *this;
    }
    virtual LRESULT Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        LRESULT lresult;
        auto procKeys=wndproc.equal_range(wp);      
        std::for_each(procKeys.first,procKeys.second,[&](decltype(wndproc)::mapped_type func){
            lresult=func(hwnd,msg,wp,lp);
        });
        return lresult;
    }
    auto& add(decltype(wndproc)::key_type msg,decltype(wndproc)::mapped_type func){
        wndproc.emplace(msg,func);
        return *this;
    }
};
class WndProcCmdWrapper:public WndProcWrapper{
    public:
    WndProcCmdWrapper(decltype(share) share):WndProcWrapper(share){

    }
    LRESULT Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
        LRESULT lresult;
        auto procKeys=wndproc.equal_range(wp);      
        std::for_each(procKeys.first,procKeys.second,[&](decltype(wndproc)::mapped_type func){
            lresult=func(hwnd,msg,wp,lp);
        });
        return lresult;
    }
};