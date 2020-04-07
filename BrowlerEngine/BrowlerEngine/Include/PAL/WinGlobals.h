#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS

class ReadOnlyWinGlobals;
#ifndef _WINDEF_
class HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
//typedef wchar_t WCHAR;
#endif

class WinGlobals BRWL_FINAL
{
public:
	friend class ReadOnlyWinGlobals;

	WinGlobals(const HINSTANCE& hInstance, const LPWSTR cmdLine, const int& cmdShow);

private:
	const HINSTANCE	hInstance;
	const LPWSTR	cmdLine;
	const int		cmdShow;
};


class ReadOnlyWinGlobals BRWL_FINAL
{
public:
	ReadOnlyWinGlobals(const WinGlobals& globals);

	const HINSTANCE& GetHInstance() const;
	const LPWSTR& GetCmdLine() const;
	const int& GetCmdShow() const;

private:
	const WinGlobals& globals;
};

BRWL_PAL_NS_END


#endif // BRWL_PLATFORM_WINDOWS
