#pragma once

// engine stuff
#include "Common/Common_Precomp.h"
#include "Core/Core_Precomp.h"

// windows app stuff
#include "framework.h"
// undef annoying defines from framework library
#ifdef DEBUG
// should be referred to as _DEBUG
#undef DEBUG
#endif
#ifdef ERROR
#undef ERROR
#endif


#define WM_USER_POST_LOG_MSG (WM_APP + 1)
