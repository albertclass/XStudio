// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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

// Common
#include "defines.h"
#include "pugixml.hpp"
#include "allocator.h"
#include "getopt.h"
#include "logger.h"
#include "debuger.h"
#include "exception.h"
#include "datetime.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "xutility.h"
#include "serialization.h"
#include "timer.h"
#include "destructor.h"
#include "ini_reader.h"
#include "csv_reader.h"

// Database
#include "database.h"

// Net
#include "Netbase.h"
#include "net_module.h"

#ifdef _WINDOWS
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib")
#endif

using namespace xgc;
using namespace xgc::net;
using namespace xgc::sql;
using namespace xgc::common;
