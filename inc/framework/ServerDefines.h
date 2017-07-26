// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "config.h"

// crt
#ifdef _WINDOWS
#   include <io.h>
#   include <conio.h>
#   include <mbstring.h>
#endif

#include <stdio.h>
#include <stdlib.h>

// msvcp
#include <functional>
#include <fstream>
#include <memory>

#ifdef _WINDOWS
#   include <xutility>
#endif

#ifdef _WINDOWS
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib")
#endif

#ifdef _LINUX
#	include<sys/wait.h>  
#	include<signal.h> 
#endif

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
#include "net.h"
#include "net_module.h"

using namespace xgc;
using namespace xgc::net;
using namespace xgc::sql;
using namespace xgc::common;
