
// Visualization2.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "Visualization2.h"
#include "Visualization2Dlg.h"

// engine headers
#include "Core/PAL/WinGlobals.h"
#include "Core/Timer.h"
#include "Core/BrowlerEngine.h"
#include "Core/ApplicationEndoints.h"

#define WM_PostLogMessage (WM_APP + 1)

namespace
{

	bool exceptionHandler(const BRWL_CHAR* test, const BRWL_CHAR* msg, bool mayIgnore)
	{
		BRWL_CHAR stackPrint[2000];
		BRWL::printStackTrace(stackPrint, test, msg, 2);

		int result = MessageBox(
			NULL, stackPrint,
			mayIgnore ? BRWL_CHAR_LITERAL("Error") : BRWL_CHAR_LITERAL("Fatal Error"),
			mayIgnore ? MB_ICONEXCLAMATION | MB_CANCELTRYCONTINUE : MB_ICONERROR | MB_OK
		);

		switch (result)
		{
		case IDCANCEL:
			return false;
		case IDCONTINUE:
			return true;
		case IDOK:
			return false;
		default:
			MessageBox(
				NULL, BRWL_CHAR_LITERAL("Unknown return value from error dialogue"),
				BRWL_CHAR_LITERAL("Error"),
				MB_ICONERROR | MB_OK
			);
			return false;
		}
	}
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVisualization2App

BEGIN_MESSAGE_MAP(CVisualization2App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVisualization2App construction

CVisualization2App::CVisualization2App()
{
	BRWL::earlyStaticInit();
	BRWL::globalExceptionHandler = exceptionHandler;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	BRWL_EXCEPTION(hInstance != NULL, BRWL_CHAR_LITERAL("No HINSTANCE returned."));
	BRWL::PAL::WinGlobals globals(
		m_hInstance,
		GetCommandLine(),
		m_nCmdShow
	);
#ifdef UNICODE
	BRWL_EXCEPTION(false, nullptr);
#endif
	BRWL_EXCEPTION(false > 'h', BRWL_CHAR_LITERAL("This is a test"));
	

	BRWL::PAL::ReadOnlyWinGlobals readOnlyGlobals(globals);

	metaEngine = std::make_unique<BRWL::MetaEngine>(&readOnlyGlobals);

}

CVisualization2App::~CVisualization2App()
{
	BRWL::lateStaticDestroy();
}


// The one and only CVisualization2App object

CVisualization2App app;


// CVisualization2App initialization

BOOL CVisualization2App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CVisualization2Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		//TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
