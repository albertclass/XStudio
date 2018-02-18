#pragma once
#ifdef _DEBUG
#	include "vld.h"
#endif
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
// stl headers
#include <list>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <filesystem>
#include <ios>
#include <ostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <xutility>

// crt headers
#include <stdio.h>
#include <process.h>

// windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>

#include <Python.h>
// For all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "wx/sysopt.h"

// thrid part headers
#include "defines.h"
#include "netbase.h"
#include "xutility.h"
#include "xsystem.h"

#include "md5.h"
#include "ini_reader.h"
#include "serialization.h"
#include "filewatcher.h"

using namespace std::placeholders;
using namespace xgc;
using namespace xgc::common;
using namespace xgc::net;
using namespace xgc::Encryption;
namespace fs = std::tr2::sys;

#include "AppConfig.h"
#include "proto_def.h"