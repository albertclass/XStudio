#include "config.h"
#if defined( __GNUC__ )
#include <string.h>

int strcpy_s( char *destination, size_t count, const char *source )
{
	if( nullptr == source )
		return -1;

	size_t i = 0;
	while( i < count - 1 && source[i] )
	{
		destination[i] = source[i];
		++i;
	}

	destination[i] = 0;

	return 0;
}

int strncpy_s( char *destination, size_t count, const char *source, size_t copied )
{
	if( nullptr == source )
		return -1;

	size_t i = 0;
	while( i < count - 1 && i < copied && source[i] )
	{
		destination[i] = source[i];
		++i;
	}

	destination[i] = 0;

	return 0;
}

#endif