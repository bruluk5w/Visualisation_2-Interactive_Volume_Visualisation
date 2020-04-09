#ifdef BRWL_PLATFORM_WINDOWS

#include "PAL/WinGlobals.h"

#include "Timer.h"
#include "BrowlerEngine.h"
#include "Resource.h"

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_ HINSTANCE		hInstance,
	_In_opt_ HINSTANCE	hPrevInstance,
	_In_ LPWSTR			lpCmdLine,
	_In_ int			nCmdShow)
{
	BRWL::PAL::WinGlobals globals (
		hInstance,
		lpCmdLine,
		nCmdShow
	);

	BRWL::PAL::ReadOnlyWinGlobals readOnlyGlobals(globals);

	BRWL::MetaEngine metaEngine = BRWL::MetaEngine(&readOnlyGlobals);
#if 1
	{


		WNDCLASSEXW wc;
		ZeroMemory(&wc, sizeof(wc)),

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.hInstance = readOnlyGlobals.GetHInstance();
		wc.hIcon = LoadIcon(readOnlyGlobals.GetHInstance(), MAKEINTRESOURCE(IDI_BROWLERENGINE));
		wc.hIconSm = LoadIcon(readOnlyGlobals.GetHInstance(), MAKEINTRESOURCE(IDI_BROWLERENGINE_SMALL));
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


	}

#endif
	// enter the main loop:
	metaEngine.initialize();

	// this struct holds Windows event messages
	MSG msg = { 0 };

	// Enter the infinite message loop
	while (msg.message != WM_QUIT)
	{
		// Check to see if any messages are waiting in the queue
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// translate keystroke messages into the right format
			TranslateMessage(&msg);
			// send the message to the WindowProc function
			DispatchMessage(&msg);
			// check to see if it's time to quit
			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			metaEngine.update();
		}
	}
	
	return (int)msg.wParam;

}


// this is the main message handler for the program
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
		// this message is read when the window is closed
		case WM_DESTROY:
		{
			// close the application entirely
			PostQuitMessage(0);
			return 0;
		} break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif // BRWL_PLATFORM_WINDOWS
