#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

#ifndef _WINDEF_
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
typedef wchar_t WCHAR;
typedef WCHAR* LPWSTR;
#endif


BRWL_PAL_NS

class ReadOnlyWinGlobals;


class WinGlobals final
{
public:
	friend class ReadOnlyWinGlobals;

	WinGlobals(const HINSTANCE& hInstance, const BRWL_CHAR* cmdLine, const int& cmdShow);

private:
	const HINSTANCE	hInstance;
	const BRWL_CHAR* cmdLine;
	const int		cmdShow;
};


class ReadOnlyWinGlobals final
{
public:
	ReadOnlyWinGlobals(const WinGlobals& globals);

	const HINSTANCE& GetHInstance() const;
	const BRWL_CHAR* GetCmdLine() const;
	const int& GetCmdShow() const;

private:
	const WinGlobals& globals;
};

BRWL_PAL_NS_END


#endif // BRWL_PLATFORM_WINDOWS
