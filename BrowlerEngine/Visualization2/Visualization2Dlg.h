#pragma once

#include "CLogEdit.h"

class LogHandler;

// CVisualization2Dlg dialog
class CVisualization2Dlg : public CDialogEx
{
// Construction
public:
	CVisualization2Dlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CVisualization2Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VISUALIZATION2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog() override;
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMainMenuBarHelpAbout();
	afx_msg void OnMainMenuBarFileExit();
	afx_msg LRESULT OnLogMessage(WPARAM, LPARAM lParam);
protected:
	// The edit control which displays the application log
	CLogEdit logEdit;
	std::unique_ptr<LogHandler> logHandler;
};
