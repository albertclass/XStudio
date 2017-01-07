#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined( WIN32 ) || defined( WIN64 )
#	define _WINDOWS
#elif defined( LINUX32 ) || defined( LINUX64 )
#	define _LINUX
#endif

#ifdef _MESSAGE_TIPS
#	ifdef _LINUX
#		pragma message( "system using linux" )
#	endif
#
#	ifdef _WINDOWS
#		pragma message( "system using windows" )
#	endif
#	ifdef _MSC_VER
#		pragma message( "compiler using visual studio" )
#	elif __GNUC__
#		pragma message( "compiler using gnuc" )
#	else
#		pragma message( "compiler using other" )
#	endif
#endif

#if __WORDSIZE == 64
#	define _AI_X32
#elif defined  __x86_64__
#	define _AI_X64
#endif

#ifdef _WINDOWS
#	include "platform/windows/sys.h"
#endif

#ifdef _LINUX
#	include "platform/linux/sys.h"
#endif


#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

// #include <map>
// #include <set>
// #include <list>
// #include <vector>
// #include <unordered_map>
// #include <unordered_set>
// #include <algorithm>
///////////////////////////////////////////////////////////
// adpater linux to windows

#if defined( __GNUC__ )
#	include "platform/linux/c++.h"
#endif //__GNUC__

#if defined( _MSC_VER )
#	include "platform/windows/c++.h"
#endif
#endif //_CONFIG_H_