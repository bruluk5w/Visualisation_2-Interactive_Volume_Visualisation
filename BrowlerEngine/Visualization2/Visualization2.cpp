
// Visualization2.cpp : Defines the class behaviors for the application.
//

#include "framework.h"
#include "Visualization2.h"
#include "Visualization2Dlg.h"
#include "Visualization2Renderer.h"

// engine headers
#include "Common/PAL/WinGlobals.h"
#include "Core/Timer.h"
#include "Core/BrowlerEngine.h"
#include "Core/Hierarchy.h"
#include "Core/ApplicationEndoints.h"
#include "Visualization2Updatable.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"

namespace
{

	bool exceptionHandler(const BRWL_CHAR* test, const BRWL_CHAR* msg, bool mayIgnore)
	{
		BRWL_CHAR stackPrint[2000];
		BRWL::printStackTrace(stackPrint, test, msg, 2);

		int result = MessageBox(
			NULL, stackPrint,
			mayIgnore ? BRWL_CHAR_LITERAL("Error") : BRWL_CHAR_LITERAL("Fatal Error"),
			mayIgnore ? MB_ICONEXCLAMATION | MB_RETRYCANCEL : MB_ICONERROR | MB_OK
		);

		switch (result)
		{
		case IDCANCEL:
			return false;
		case IDRETRY:
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
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVisualization2App construction

CVisualization2App::CVisualization2App() :
	metaEngine(nullptr),
	globals(nullptr),
	readOnlyGlobals(nullptr)
{
	BRWL::earlyStaticInit();
	BRWL::globalExceptionHandler = exceptionHandler;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	BRWL_EXCEPTION(hInstance != NULL, BRWL_CHAR_LITERAL("No HINSTANCE returned."));

	globals = std::make_unique<BRWL::PAL::WinGlobals>(m_hInstance, GetCommandLine(), m_nCmdShow);
	readOnlyGlobals = std::make_unique<BRWL::PAL::ReadOnlyWinGlobals>(*globals);

	metaEngine = std::make_unique<BRWL::MetaEngine>(readOnlyGlobals.get());
}

CVisualization2App::~CVisualization2App()
{
	BRWL::lateStaticDestroy();
}


// The one and only CVisualization2App object

CVisualization2App app;

void CVisualization2App::init()
{
	metaEngine->initialize();
	BRWL::MetaEngine::EngineHandle handle = metaEngine->getDefaultEngineHandle();
	metaEngine->setEngineRunMode(handle, BRWL::MetaEngine::EngineRunMode::DETATCHED);
	BRWL::Engine* engine = metaEngine->getEngine(handle);
	engine->createUpdatable<::BRWL::Visualization2Updatable>();
	BRWL_EXCEPTION(engine->renderer != nullptr, BRWL_CHAR_LITERAL("Renderer not set up."));
	engine->renderer->createAppRenderer<BRWL::RENDERER::Visualization2Renderer>();
}

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

	init();

	INT_PTR nResponse = dlg.DoModal();  //<-- starts the console and calls the first update which spawns the thread for the engine loop
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

int CVisualization2App::ExitInstance()
{
	return CWinApp::ExitInstance();
}

void CVisualization2App::OpenFile(const BRWL_CHAR* file)
{
	metaEngine->isInitialized();
	BRWL::MetaEngine::EngineHandle handle = metaEngine->getDefaultEngineHandle();
	BRWL::Engine* engine = metaEngine->getEngine(handle);
	BRWL_EXCEPTION(engine->renderer != nullptr, BRWL_CHAR_LITERAL("Renderer not set up."));
	BRWL::RENDERER::Visualization2Renderer* renderer = dynamic_cast<BRWL::RENDERER::Visualization2Renderer*>(engine->renderer->getAppRenderer());
	if (renderer != nullptr) {
		renderer->setFilePath(file);
	}
}


