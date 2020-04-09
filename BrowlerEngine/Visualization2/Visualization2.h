
// Visualization2.h : main header file for the PROJECT_NAME application
//

#pragma once
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

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

// Added stuff:
public:
	BRWL::MetaEngine* GetMetaEngine() const { BRWL_EXCEPTION(metaEngine != nullptr, "MetaEngine is nullptr!"); return metaEngine.get(); }

protected:
	std::unique_ptr<BRWL::MetaEngine> metaEngine;
};

extern CVisualization2App app;
