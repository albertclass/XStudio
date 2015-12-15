#ifdef _LIB_EXPORTS
# ifdef _USRDLL
#	define COMMON_API __declspec(dllexport)
#	define __LZO_EXPORT1 __declspec(dllexport)
# elif defined( _LIB )
#	define COMMON_API
# endif
#else
# ifdef _COMMON_STATIC
#	define COMMON_API
# else
#	define COMMON_API __declspec(dllimport)
#	define __LZO_EXPORT1 __declspec(dllimport)
# endif
#endif
