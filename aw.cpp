#include <windows.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <functional>
namespace h {
    enum WM_CMD {
        BTN_PUSH=4649
    };
    template <class BT, class CT>
    class ObjectManager {
    protected:
        BT base;
        CT created;
    public:
        ObjectManager(BT base) :base(base) {

        }
        virtual ~ObjectManager() {}
        virtual const BT& getBase() {
            return base;
        }
        virtual const CT& getCreated() {
            return created;
        }

    };
    class PaintManager :public ObjectManager<HWND,HDC>{
    private:
        PAINTSTRUCT ps;
    public:
        PaintManager(HWND hwnd):ObjectManager(hwnd) {
            created=BeginPaint(base, &ps);
        }
        ~PaintManager() {
            EndPaint(base, &ps);
        }
    };
    class ColorManager :public ObjectManager<COLORREF, HBRUSH> {
    private:
    public:
        ColorManager(COLORREF color) :ObjectManager(color) {
            created = CreateSolidBrush(color);
        }
        ~ColorManager() {
            if (created == nullptr)return;
            DeleteObject(created);
        }
    };
    class FontManager :public ObjectManager<std::wstring, HFONT> {
    public:
        auto& reset(int height,int width) {
            created = CreateFont(height, width, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, base.c_str());
            return *this;
        }
        FontManager(std::wstring fontName,int height,int width):ObjectManager(fontName) {
            reset(height,width);
        }
        ~FontManager() {
            if (created == nullptr)return;
            DeleteObject(created);
        }
    };

    inline auto getWindowStr(HWND hwnd) {
        std::wstring str;
        str.resize(GetWindowTextLength(hwnd)+1);
        GetWindowText(hwnd, &str[0],str.size());
        return str;
    }
    inline auto baseStyle(WNDPROC wndproc, LPCWSTR name) {
        WNDCLASS winc;
        winc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        winc.cbClsExtra = winc.cbWndExtra = 0;
        winc.hInstance = (HINSTANCE)GetModuleHandle(0);
        winc.hIcon = LoadIcon(winc.hInstance, IDI_APPLICATION);
        winc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        winc.lpszMenuName = nullptr;
        winc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        winc.lpfnWndProc = wndproc;
        winc.lpszClassName = name;
        return RegisterClass(&winc);
    }
    class MySystem {
    public:
        static auto& defColor() {
            static ColorManager color(RGB(0, 255, 0));//255&bool^get
            return color;
        }
        static auto& defColorX() {
            static ColorManager color(255 ^ defColor().getBase());
            return color;
        }
        static auto& bkColor() {
            static ColorManager color(RGB(0, 0, 0));
            return color;
        }
    };
    class Doing {
    public:
        Doing(std::function<void()> function) {
            function();
        }
    };
    
    enum class mouseUpTimerControl :UINT {
        UP,
        INTERVAL=100
    };

    class mouseUpTimer {
    private:
        static std::unordered_map < UINT, std::pair<std::function<void(HWND, UINT, UINT, DWORD)>, std::function<void(HWND, UINT, UINT, DWORD)> >  > data;
        static VOID CALLBACK timer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
            if (0 <= GetAsyncKeyState(VK_LBUTTON)) {
                data[idEvent].second(hwnd, uMsg, idEvent, dwTime);
                KillTimer(hwnd, idEvent);
                return;
            }
           data[idEvent].first(hwnd, uMsg, idEvent, dwTime);
        }
        mouseUpTimer() {}
    public:
        mouseUpTimer(const mouseUpTimer&) = delete;
        mouseUpTimer& operator=(const mouseUpTimer&) = delete;
        mouseUpTimer(const mouseUpTimer&&) = delete;
        mouseUpTimer& operator=(const mouseUpTimer&&) = delete;
        static auto &getInstance() {
            static mouseUpTimer instance;
            return instance;
        }
        auto &set(const UINT id,const typename decltype(data)::mapped_type::first_type first,decltype(first) second) {
            data[id] = { first,second };
            return getInstance();
        }
        auto& setTimer(const HWND hwnd,const UINT id,const UINT interval) {
            SetTimer(hwnd,id,interval,(TIMERPROC)timer);
            return getInstance();
        }
    };
    decltype(mouseUpTimer::data) mouseUpTimer::data;
    class Cracker {
    public:
        inline virtual LRESULT cracker(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) = 0;
    };
    class WndProc :public Cracker{
    private:
        std::unordered_multimap<UINT, Cracker* > crackers;
        typename decltype(crackers)::mapped_type def;
    public:
        inline WndProc(decltype(def) def):def(def) {

        }
        inline auto& add(typename decltype(crackers)::key_type msg,typename decltype(crackers)::mapped_type cracker) {
            crackers.emplace(msg, cracker);
            return *this;
        }
        inline LRESULT cracker(HWND hwnd , UINT msg, WPARAM wp, LPARAM lp)override {
            if (crackers.count(msg) == 0) {
                return def->cracker(hwnd,msg,wp,lp);
            }
            auto iters=crackers.equal_range(msg);
            LRESULT lresult;
            for (auto crack = iters.first; crack != iters.second; ++crack) {
                lresult=crack->second->cracker(hwnd,msg,wp,lp);
            }
            return lresult;
        }
    };
    class Exit:public Cracker{
    public:
        inline LRESULT cracker(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)override {
            PostQuitMessage(0);
            return DefWindowProc(hwnd, msg, wp, lp);
        }
    };
    class Def :public Cracker {
    public:
        inline LRESULT cracker(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)override {
            return DefWindowProc(hwnd, msg, wp, lp);
        }
    };
    class CreateMessage :public Cracker {
        inline LRESULT cracker(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)override {
            MessageBox(nullptr, L"", L"", 0);
            return DefWindowProc(hwnd, msg, wp, lp);
        }
    };
    class Window {
    private:
        static WndProc* lastCreated;
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
            return ((WndProc*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->cracker(hwnd, msg, wp, lp);
        }
        static LRESULT CALLBACK createWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lastCreated);
            SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)windowProc);
            return lastCreated->cracker(hwnd, msg, wp, lp);
        }

        static Doing doing;
        static constexpr auto WINDOW_CLASS_NAME = TEXT("WNDPROC_OF_WINDOW_CLASS"),
                              CREATE_CLASS_NAME=TEXT("CREATE_WNDPROC_OF WINDOW_CLASS");
    public:
        Window(WndProc *wndProc,LPCWSTR title,DWORD style,RECT rect,HWND parent,HMENU menu,HINSTANCE hInstance) {
            lastCreated = wndProc;
            //SetWindowLongPtr(
            CreateWindow(CREATE_CLASS_NAME, title, style, rect.left, rect.top, rect.right, rect.bottom, parent, menu, hInstance, nullptr);
            //,GWLP_USERDATA, (LONG_PTR)wndProc);
        }
        static auto messageLoop() {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            return msg.wParam;
        }

    };
    decltype(Window::lastCreated) Window::lastCreated;
    decltype(Window::doing) Window::doing([]{
        baseStyle(createWindowProc, CREATE_CLASS_NAME);
        baseStyle(windowProc, WINDOW_CLASS_NAME);
        });
};
//VOID CALLBACK mouseUpTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
//    if (0<=GetAsyncKeyState(VK_LBUTTON)) {
//        SendMessage(hwnd, WM_LBUTTONUP, 0, 0);
//        KillTimer(hwnd, idEvent);
//        return;
//    }
//}
//LRESULT CALLBACK btnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//    static std::unordered_map<HWND, bool> data;
//    switch (msg) {
//    case WM_CREATE:
//        //data.emplace(hwnd);
//        break;
//    case WM_DESTROY:
//        data.erase(hwnd);
//        break;
//    case WM_LBUTTONUP:
//        data[hwnd] = false;
//        InvalidateRect(hwnd, nullptr, true);
//        UpdateWindow(hwnd);
//        if (GetParent(hwnd) == nullptr)break;
//        SendMessage(GetParent(hwnd), WM_COMMAND, h::WM_CMD::BTN_PUSH, 0);
//        break;
//    case WM_LBUTTONDOWN:
//        data[hwnd] = true;
//        h::mouseUpTimer::getInstance().setTimer(hwnd, static_cast<UINT>(h::mouseUpTimerControl::UP), static_cast<UINT>(h::mouseUpTimerControl::INTERVAL));
//        InvalidateRect(hwnd, nullptr, true);
//        UpdateWindow(hwnd);
//        break;
//    case WM_PAINT:
//    {
//        RECT rect;
//        SIZE fontSize;
//        auto text = h::getWindowStr(hwnd);
//        GetClientRect(hwnd, &rect);
//        h::PaintManager paint(hwnd);
//        h::FontManager font(L"游明朝", rect.bottom,rect.right/text.size());
//        SelectObject(paint.getCreated(), font.getCreated());
//        SetBkColor(paint.getCreated(), h::MySystem::bkColor().getBase());
//        SetTextColor(paint.getCreated(), data[hwnd] ? h::MySystem::defColorX().getBase() : h::MySystem::defColor().getBase());
//        DrawText(paint.getCreated(), text.c_str(), -1, &rect, DT_CENTER | DT_WORDBREAK);
//        FrameRect(paint.getCreated(), &rect,data[hwnd]? h::MySystem::defColorX().getCreated() : h::MySystem::defColor().getCreated());
//    }
//        break;
//    }
//    return DefWindowProc(hwnd, msg, wp, lp);
//}
//LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//    switch (msg) {
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    case WM_CREATE:
//        CreateWindow(TEXT("btn"), TEXT("BTN"), WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 100, 100, hwnd, nullptr, LPCREATESTRUCT(lp)->hInstance, nullptr);
//        break;
//    }
//    return DefWindowProc(hwnd, msg, wp, lp);
//}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdLine,int nCmdShow) {
    h::Def def;
    h::Exit exit;
    h::CreateMessage create;
    h::WndProc wndproc(&def);
    wndproc
        .add(WM_DESTROY,&exit)
        .add(WM_CREATE,&create);
    h::Window window(&wndproc,TEXT("test"),WS_VISIBLE|WS_OVERLAPPEDWINDOW,{0,0,100,100},nullptr,nullptr,hInstance);
    window.messageLoop();
	//MSG msg;
 //   h::baseStyle(wndProc, L"main");
 //   h::baseStyle(btnProc,L"btn");
 //   h::mouseUpTimer::getInstance().set(static_cast<UINT>(h::mouseUpTimerControl::UP),
 //       [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {},
 //       [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
 //           SendMessage(hwnd, WM_LBUTTONUP, 0, 0);
 //       }
 //   );
 //   CreateWindow(TEXT("main"), TEXT("Main"), WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);
	//while (GetMessage(&msg, nullptr, 0, 0)) {
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}
	return 0;
}