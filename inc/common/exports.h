#if defined( _WINDOWS )
#	ifdef _LIB_EXPORTS
#	 ifdef _DLL
#		define COMMON_API __declspec(dllexport)
#		define __LZO_EXPORT1 __declspec(dllexport)
#	 elif defined( _LIB )
#		define COMMON_API
#	 endif
#	else
#	 ifdef _COMMON_STATIC
#		define COMMON_API
#	 else
#		define COMMON_API __declspec(dllimport)
#		define __LZO_EXPORT1 __declspec(dllimport)
#	 endif
#	endif
#elif defined( __GNUC__ )
#	if defined( _LIB_EXPORTS ) && defined( _DLL )
#		define COMMON_API __attribute__((__visibility__("default")))
#		define __LZO_EXPORT1 __attribute__((__visibility__("default")))
#	else
#		define COMMON_API
#	endif
#else 
#	define COMMON_API
#endif