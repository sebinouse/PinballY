// This file is part of PinballY
// Copyright 2018 Michael J Roberts | GPL v3 or later | NO WARRANTY
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <comdef.h>

// C/C++ standard headers
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <string>

// widely used local headers
#include "../Utilities/Util.h"
#include "../Utilities/WinUtil.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/LogError.h"
#include "../Utilities/InstanceHandle.h"
