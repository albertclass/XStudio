// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <conio.h>
#include <mbstring.h>
#include <string.h>

#include <chrono>
#include <filesystem>
#include <ios>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <xutility>

namespace fs = std::tr2::sys;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Python.h>

// TODO: reference additional headers your program requires here
#include "defines.h"
#include "timer.h"
#include "exception.h"
#include "datetime.h"
#include "xsystem.h"
#include "xutility.h"
#include "ini_reader.h"
#include "csv_reader.h"
#include "console.h"
#include "serialization.h"
#include "pugixml.hpp"

#include "netbase.h"

using namespace xgc;
using namespace xgc::net;
using namespace xgc::common;