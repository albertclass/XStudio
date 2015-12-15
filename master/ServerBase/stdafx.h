// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
// Windows Header Files:
#include <windows.h>

// crt
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
// msvcp
#include <functional>
#include <fstream>
#include <filesystem>
#include <memory>
#include <xutility>

// window sdk
#include <WinSock2.h>

// SGDP
#include "sdbase.h"
#include "sdnet.h"
#include "sdnetopt.h"
#include "sdpipe.h"
#include "sdgate.h"

// Common
#include "defines.h"
#include "ini.h"
#include "markupstl.h"
#include "allocator.h"
#include "getopt.h"
#include "logger.h"
#include "shmbuf.h"
#include "debuger.h"
#include "exception.h"
#include "datetime.h"
#include "xstring.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "xutility.h"
#include "freelist.h"
#include "serialization.h"
#include "timer_manager.h"
#include "destructor.h"

// Database
#include "Database.h"

// proto
#include "netdata.h"
#include "protoheader.h"
#include "protoenums.h"

using namespace XGC;
using namespace XGC::common;