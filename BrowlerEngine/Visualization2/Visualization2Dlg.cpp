
// Visualization2Dlg.cpp : implementation file
//

#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"
#include "LogHandler.h"

#include "pch.h"
#include "framework.h"
#include "Visualization2.h"
#include "Visualization2Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CVisualization2Dlg dialog



CVisualization2Dlg::CVisualization2Dlg(CWnd* pParent /*=nullptr*/) :
	CDialogEx(IDD_VISUALIZATION2_DIALOG, pParent),
	logHandler(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CVisualization2Dlg::~CVisualization2Dlg()
{ }

void CVisualization2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_EDIT, logEdit);
}

BEGIN_MESSAGE_MAP(CVisualization2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_HELP_ABOUT, &CVisualization2Dlg::OnMainMenuBarHelpAbout)
	ON_COMMAND(ID_FILE_EXIT, &CVisualization2Dlg::OnMainMenuBarFileExit)
	ON_MESSAGE(WM_USER_POST_LOG_MSG, &CVisualization2Dlg::OnLogMessage)
END_MESSAGE_MAP()


// CVisualization2Dlg message handlers

BOOL CVisualization2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_SHOW); // SW_MAXIMIZE

	// TODO: Add extra initialization here
	{
		logHandler = std::make_unique<LogHandler>(*this);
		BRWL::MetaEngine* meta = app.GetMetaEngine();
		BRWL::MetaEngine::EngineHandle defaultHandle = meta->getDefaultEngineHandle();
		BRWL::Engine* defaultEngine =  meta->getEngine(defaultHandle);
		defaultEngine->logger->setOutStream(logHandler.get());
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVisualization2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVisualization2Dlg::OnPaint()
{
	app.GetMetaEngine()->update();

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVisualization2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVisualization2Dlg::OnDestroy()
{
	// TODO: wait for end of frame!
	BRWL::MetaEngine* meta = app.GetMetaEngine();
	BRWL::Engine* defaultEngine = meta->getEngine(meta->getDefaultEngineHandle());
	defaultEngine->logger->setOutStream(nullptr);
	CDialogEx::OnDestroy();
}


void CVisualization2Dlg::OnMainMenuBarHelpAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CVisualization2Dlg::OnMainMenuBarFileExit()
{
	app.GetMetaEngine()->shutDown();
	EndDialog(IDOK);
}

LRESULT CVisualization2Dlg::OnLogMessage(WPARAM, LPARAM lParam)
{
	BRWL_CHECK(logHandler != nullptr, BRWL_CHAR_LITERAL("No logger present!"));
	if (logHandler) {
		const BRWL_STR& msg = logHandler->getStringRef(lParam);
		logEdit.OnLogMessage(msg);
		logHandler->setStringDone(lParam);
	}
	
	return 0; // doesn't matter actually
}
