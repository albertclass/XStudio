#pragma once
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// msvcp
#include <functional>
#include <fstream>
#include <memory>
#include <sstream>

#ifdef _WINDOWS
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib")
#endif

#include "defines.h"
#include "xvariant.h"

using namespace xgc;
using namespace xgc::common;

// redis cli library header file.
#include "hiredis.h"