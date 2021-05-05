#pragma once // (c) 2020 Lukas Brunner

#include "Common/GlobalsFwd.h"

BRWL_NS
class MetaEngine;
BRWL_NS_END


#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVisualization2App:
// See Visualization2.cpp for the implementation of this class
//

class CVisualization2App : public CWinApp
{
public:
	CVisualization2App();
	~CVisualization2App();

// Overrides
public:
	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override; 

// Implementation

	DECLARE_MESSAGE_MAP()

// Added stuff:
public:
	BRWL::MetaEngine* GetMetaEngine() const { BRWL_EXCEPTION(metaEngine != nullptr, BRWL_CHAR_LITERAL("MetaEngine is nullptr!")); return metaEngine.get(); }
	void OpenFile(const BRWL_CHAR* file);

protected:
	void init();

	std::unique_ptr<BRWL::MetaEngine> metaEngine;

	std::unique_ptr<BRWL::PAL::WinGlobals> globals;
	std::unique_ptr<BRWL::PAL::ReadOnlyWinGlobals> readOnlyGlobals;
};

extern CVisualization2App app;
