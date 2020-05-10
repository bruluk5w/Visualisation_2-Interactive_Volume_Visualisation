#include "PAL/WinWindow.h"

#include "PAL/BrowlerWindowsInclude.h"


namespace
{
    const BRWL_CHAR windowClassName[] = BRWL_CHAR_LITERAL("EngineDefaultWindow");
}


BRWL_PAL_NS

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    WinWindow* window;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;

        window = static_cast<WinWindow*>(createStruct->lpCreateParams);

        if (!BRWL_VERIFY(SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)(window)) != 0, BRWL_CHAR_LITERAL("Failed to set user pointer for hwnd.")))
        {
            return FALSE;
        }
    }
    else
    {
        window = (WinWindow*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    }

    window->handleMessage(msg, wParam, lParam);
}

struct WinWindowImpl
{
    WinWindowImpl(WinWindow* wrapper, int x, int y, int width, int height) :
        wrapper(wrapper)
    {
        // for now we only consider the main monitor
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        RECT rect = { x, y, width, height };
        ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

        hWnd = CreateWindowEx(
            0,
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
            wrapper // pass user pointer to this instance, so we can retrieve it again when handling WM_NCCREATE
        );

        ::GetWindowRect(hWnd, &rect);
        x = rect.left;
        y = rect.top;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    HWND hWnd;
    int width;
    int height;
    int x;
    int y;
    WinWindow* wrapper;
};


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

WinWindow::WinWindow(PlatformGlobals* globals) :
    globals(globals),
    impl(nullptr)
{
    RegisterWindowClass(globals->GetHInstance(), windowClassName);
}

void WinWindow::create()
{
    impl = std::make_unique<WinWindowImpl>();
}

void WinWindow::destroy()
{
    impl = nullptr;
}

int WinWindow::width() { return BRWL_VERIFY(impl, BRWL_CHAR_LITERAL("Window not created yet!")) ? impl->width : -1; }

LRESULT WinWindow::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (impl)
    {
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
            RECT clientRect;
            GetClientRect(impl->hWnd, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            Resize(width, height);
        }
        break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProcW(impl->hWnd, msg, wParam, lParam);
        }
    }
    else
    {
        return ::DefWindowProcW(impl->hWnd, msg, wParam, lParam);
    }

    return 0;
}

BRWL_PAL_NS_END
