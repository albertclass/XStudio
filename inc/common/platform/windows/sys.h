#ifndef __XGC_SYSTEM_H__
#define __XGC_SYSTEM_H__

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <Windows.h>
#include <stdlib.h>
#include <conio.h>

#define XGC_MAX_PATH 	_MAX_PATH
#define XGC_MAX_FNAME 	_MAX_FNAME

#define xgc_invalid_handle INVALID_HANDLE_VALUE

#define gettid() GetCurrentThreadId()

#endif // __XGC_SYSTEM_H__