#pragma once


// CLogEdit

class CLogEdit : public CEdit
{
	DECLARE_DYNAMIC(CLogEdit)

public:
	const unsigned int maxLogLimit = 40000;
	CLogEdit();
	virtual ~CLogEdit();

	void Init();

	void OnLogMessage(const BRWL_STR& msg);

protected:
	DECLARE_MESSAGE_MAP()
};


