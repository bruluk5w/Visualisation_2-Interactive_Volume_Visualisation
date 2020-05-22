#include "PAL/WinWindow.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "Common/PAL/BrowlerWindowsInclude.h"

#include "BrowlerEngine.h"
#include "Timer.h"


namespace
{
    const BRWL_CHAR windowClassName[] = BRWL_CHAR_LITERAL("EngineDefaultWindow");
    void getScreenSpaceClientRect(HWND& hWnd, RECT& rect) {
        GetClientRect(hWnd, &rect);
        POINT origin = { rect.left, rect.top };
        ClientToScreen(hWnd, &origin);
        rect.left += origin.x;
        rect.top += origin.y;
        rect.right += origin.x;
        rect.bottom += origin.y;
    }
}


BRWL_PAL_NS

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct WinWindowImpl
{
    WinWindowImpl(WinWindow* wrapper, int x, int y, int width, int height) :
        x(0),
        y(0),
        width(-1),
        height(-1),
        hWnd(),
        lastTimeTransformUpdated(-1),
        wrapper(wrapper)
    {
        // for now we only consider the main monitor
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        RECT rect = { x, y, width, height };
        ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

        hWnd = CreateWindowEx(
            WS_EX_APPWINDOW,
            windowClassName,    // name of the window class
            L"Put Title Here",   // title of the window
            WS_OVERLAPPEDWINDOW,    // window style
            rect.left,    // x-position of the window
            rect.top,    // y-position of the window
            rect.right - rect.left,    // width of the window
            rect.bottom - rect.top,    // height of the window
            NULL,    // we have no parent window, NULL
            NULL,    // we aren't using menus, NULL
            wrapper->globals->GetHInstance(),    // application handle
            this // pass user pointer to this instance, so we can retrieve it again when handling WM_NCCREATE
        );

        BRWL_EXCEPTION(hWnd, BRWL_CHAR_LITERAL("Failed to create Window!"));
        if (!hWnd) {
            PAL::ShowLastWindowsError();
        }

        getScreenSpaceClientRect(hWnd, rect);
        x = rect.left;
        y = rect.top;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        //ShowWindow(hWnd, wrapper->globals->GetCmdShow());
        ShowWindow(hWnd, SW_SHOW);
    }

    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg)
        {
        case WM_PAINT:
            //Update();
            //Render();
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {

            switch (wParam)
            {
            case VK_ESCAPE:
                ::PostQuitMessage(0);
                break;
            case VK_F11:
                //SetFullscreen(!g_Fullscreen);
                break;
            }
        }
        break;

        case WM_SYSCHAR:
            // Handle this one cause else windows playes the annoying PLINNNG sound
            break;
        case WM_SIZE:
        {
            float currentTime = engine->time->getUnmodifiedTimeF();
            float deltaUpdate = currentTime - lastTimeTransformUpdated;
            if (deltaUpdate > 0.2) {
                OnChangeTransform();
                lastTimeTransformUpdated = currentTime;
            }
        }
        break;
        case WM_ENTERSIZEMOVE:
            lastTimeTransformUpdated = engine->time->getUnmodifiedTimeF();
            break;
        case WM_EXITSIZEMOVE:
            if (lastTimeTransformUpdated < engine->time->getUnmodifiedTime()) OnChangeTransform();
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void ProcessWindowsMessages() {
        MSG msg = { 0 };

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // translate keystroke messages into the right format
            TranslateMessage(&msg);
            // send the message to the WindowProc function
            DispatchMessage(&msg);
            // check for application exit
            if (msg.message == WM_QUIT)
                int returnCode = (int)msg.wParam;
                break;
        }
    }

    void OnChangeTransform()
    {
        RECT rect;
        getScreenSpaceClientRect(hWnd, rect);
        if (rect.left != x || rect.top != y)
        {
            int dx = rect.left - x;
            int dy = rect.top - y;
            x = rect.left;
            y = rect.top;
            wrapper->move(x, y, dx, dy);
        }

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        if (this->width != width || this->height != height)
        {
            this->width = width;
            this->height = height;
            wrapper->resize(width, height);
        }
    }

    int y;
    int x;
    int width;
    int height;
    HWND hWnd;

    float lastTimeTransformUpdated;

    WinWindow* wrapper;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WinWindowImpl* impl = (WinWindowImpl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    ;
    switch (msg) {
#pragma region Messages that could be sent before WM_NCCREATE
    case WM_GETMINMAXINFO:
        return 0;
    case WM_NCDESTROY:
    case WM_DWMNCRENDERINGCHANGED:
    case WM_ACTIVATEAPP:
    case WM_NCCALCSIZE:
        if (!impl) {
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }
#pragma endregion
    case WM_CREATE:  // intentional fall-through
    case WM_NCCREATE:
    {
        CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
        if (!impl) {
            impl = static_cast<WinWindowImpl*>(createStruct->lpCreateParams);
            SetLastError(0);
            if (!BRWL_VERIFY(SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(impl)) != 0 || GetLastError() == 0, BRWL_CHAR_LITERAL("Failed to set user pointer for hwnd.")))
            {
                return FALSE;
            }
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    }
    

    BRWL_CHECK(impl, BRWL_CHAR_LITERAL("Missed a Windows message before Window creation."));
    if (!impl)
    {
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return impl->handleMessage(msg, wParam, lParam);
}

void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName)
{
    // Register a window class for creating our render window with.
    WNDCLASSEXW windowClass;

    ZeroMemory(&windowClass, sizeof(windowClass));

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInst;
    windowClass.hIcon = ::LoadIcon(hInst, NULL);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName;
    windowClass.hIconSm = ::LoadIcon(hInst, NULL);

    ATOM atom = ::RegisterClassExW(&windowClass);
    BRWL_EXCEPTION(atom > 0, nullptr);
}

WinWindow::WinWindow(PlatformGlobals* globals, EventBusSwitch<Event>* eventSystem) :
    globals(globals),
    eventSystem(eventSystem),
    impl(nullptr)
{
    BRWL_EXCEPTION(eventSystem, BRWL_CHAR_LITERAL("The EventSystem must not be nullptr."));
    RegisterWindowClass(globals->GetHInstance(), windowClassName);
}

WinWindow::~WinWindow()
{ }

void WinWindow::create(int x, int y, int width, int height) { impl = std::make_unique<WinWindowImpl>(this, x, y, width, height); }

void WinWindow::destroy() { impl = nullptr; }

int WinWindow::x() const { return BRWL_VERIFY(impl, BRWL_CHAR_LITERAL("Window not created yet!")) ? impl->x : 0; }
int WinWindow::y() const { return BRWL_VERIFY(impl, BRWL_CHAR_LITERAL("Window not created yet!")) ? impl->y : 0; }
int WinWindow::width() const { return BRWL_VERIFY(impl, BRWL_CHAR_LITERAL("Window not created yet!")) ? impl->width : -1; }
int WinWindow::height() const { return BRWL_VERIFY(impl, BRWL_CHAR_LITERAL("Window not created yet!")) ? impl->height : -1; }

void WinWindow::processPlatformMessages()
{
    if (impl) impl->ProcessWindowsMessages();
}

void WinWindow::move(int x, int y, int dx, int dy)
{
    WindowMoveParam param{ x, y, dx, dy };
    eventSystem->postEvent<Event::WINDOW_MOVE>(&param);
}

void WinWindow::resize(int width, int height)
{
    WindowSizeParam param{ width, height };
    eventSystem->postEvent<Event::WINDOW_RESIZE>(&param);
}


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS
