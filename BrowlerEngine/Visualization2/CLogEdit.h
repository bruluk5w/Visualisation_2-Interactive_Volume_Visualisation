#pragma once


// CLogEdit

class CLogEdit : public CEdit
{
	DECLARE_DYNAMIC(CLogEdit)

public:
	CLogEdit();
	virtual ~CLogEdit();

	void OnLogMessage(const BRWL_STR& msg) {
		int index = GetWindowTextLength();
		//SetFocus(); // set focus
		SetSel(index, index);
		ReplaceSel(msg.c_str());
	}

protected:
	DECLARE_MESSAGE_MAP()
};


