#if defined( __GNUC__ )
#include <string.h>

int strcat_s( char *destination, size_t size, const char *source )
{
	if( nullptr == source )
		return -1;

	size_t i = strlen(destination);
	size_t p = 0;
	while( i < size - 1 && source[p] )
	{
		destination[i] = source[p];
		++i;
		++p;
	}

	destination[i] = '\0';

	return 0;
}

int strncat_s( char *destination, size_t size, const char *source, size_t count )
{
	if( nullptr == source )
		return -1;

	size_t i = strlen( destination );
	size_t c = count;
	size_t p = 0;
	while( i < size - 1 && c && source[p] )
	{
		destination[i] = source[p];
		++i;
		--c;
		++p;
	}

	destination[i] = '\0';

	return 0;
}

#endif