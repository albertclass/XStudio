// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <winsock2.h>	// ����mysql.h��δ����__LCC__�������Ҫ�������winsock.h

#include <string>
#include <queue>
#include <process.h>

#include "mysql.h"
#include "errmsg.h"

#include "defines.h"
#include "logger.h"