#ifdef BRWL_PLATFORM_WINDOWS

#include "PAL/WinRenderer.h"

BRWL_NS

WNDCLASSEXW wc;
ZeroMemory(&wc, sizeof(wc)),

wc.cbSize = sizeof(WNDCLASSEX);
wc.style = CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc = WndProc;
wc.hInstance = globalsAcc.GetHInstance();
wc.hIcon = LoadIcon(globalsAcc.GetHInstance(), MAKEINTRESOURCE(IDI_BROWLERENGINE));
wc.hIconSm = LoadIcon(globalsAcc.GetHInstance(), MAKEINTRESOURCE(IDI_BROWLERENGINE_SMALL));
wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
wc.lpszMenuName = MAKEINTRESOURCEW(IDC_BROWLERENGINE);
wc.lpszClassName = L"MyWindow";

RegisterClassExW(&wc);
RECT wr = { 0, 0, 500, 400 };    // set the size, but not the position
AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

HWND hWnd = CreateWindowEx(NULL,
	L"MyWindow",    // name of the window class
	L"Our First Windowed Program",   // title of the window
	WS_OVERLAPPEDWINDOW,    // window style
	300,    // x-position of the window
	300,    // y-position of the window
	wr.right - wr.left,    // width of the window
	wr.bottom - wr.top,    // height of the window
	NULL,    // we have no parent window, NULL
	NULL,    // we aren't using menus, NULL
	hInstance,    // application handle
	NULL);    // used with multiple windows, NULL

if (!hWnd)
{
	return FALSE;
}

ShowWindow(hWnd, nCmdShow);

BRWL_NS_END

#endif
