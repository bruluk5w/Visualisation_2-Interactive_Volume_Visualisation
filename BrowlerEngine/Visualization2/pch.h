#pragma once

// windows app stuff
#include "framework.h"

// undef annoying defines from framework library
#ifdef DEBUG
#undef DEBUG // should be referred to as _DEBUG
#endif
#ifdef ERROR
#undef ERROR
#endif
#ifdef min
#undef min 
#endif
#ifdef max
#undef max 
#endif

// engine stuff
#include "Common/Common_Precomp.h"
#include "Core/Core_Precomp.h"




#define WM_USER_POST_LOG_MSG (WM_APP + 1)
