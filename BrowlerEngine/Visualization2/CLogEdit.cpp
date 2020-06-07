#include "CLogEdit.h"

#include "Visualization2.h"

namespace
{
	const BRWL_CHAR* oldLogDeletedHint = BRWL_CHAR_LITERAL("...older Log truncated...\r\n");
}

// CLogEdit

IMPLEMENT_DYNAMIC(CLogEdit, CEdit)

CLogEdit::CLogEdit()
{ }

CLogEdit::~CLogEdit()
{ }

void CLogEdit::Init()
{
	SetLimitText(maxLogLimit);
}

void CLogEdit::OnLogMessage(const BRWL_STR & msg)
{
	int index = GetWindowTextLength();
	if (index + msg.size() >= maxLogLimit)
	{
		SetSel(0, BRWL::Utils::max<int>(msg.size(), BRWL::Utils::min<int>(maxLogLimit / 10, msg.size() * 100)));
		ReplaceSel(oldLogDeletedHint);
	}

	index = GetWindowTextLength();

	//SetFocus(); // set focus
	SetSel(index, index);
	ReplaceSel(msg.c_str());
}



BEGIN_MESSAGE_MAP(CLogEdit, CEdit)
END_MESSAGE_MAP()


// CLogEdit message handlers


