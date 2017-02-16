// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef _DATABASE_CONFIG_
#define _DATABASE_CONFIG_
#include "defines.h"

#ifdef _WINDOWS
#	include <mysql.h>
#	include <errmsg.h>
#endif

#ifdef _LINUX
#	include <mysql/mysql.h>
#	include <mysql/errmsg.h>
#endif

#include "logger.h"
#if defined( _WINDOWS )
#	ifdef _DB_EXPORTS
#	 ifdef _DLL
#		define DATABASE_API __declspec(dllexport)
#	 else
#		define DATABASE_API
#	 endif
#	else
#	 ifdef _DB_STATIC
#		define DATABASE_API
#	 else
#		define DATABASE_API __declspec(dllimport)
#	 endif
#	endif
#elif defined( __GNUC__ )
#	if defined( _DB_EXPORTS ) && defined( _DLL )
#		define DATABASE_API __attribute__((__visibility__("default")))
#	else
#		define DATABASE_API
#	endif
#else 
#	define DATABASE_API
#endif
#endif // _DATABASE_CONFIG_