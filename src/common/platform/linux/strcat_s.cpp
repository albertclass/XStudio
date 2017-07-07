#ifdef _LINUX
#include <string.h>

errno_t strcat_s( char *destination, size_t count, const char *source )
{
	if( nullptr == source )
		return -1;

	size_t i = strlen(destination);
	size_t p = 0;
	while( i < count - 1 && source[p] )
	{
		destination[i] = source[p];
		++i;
		++p;
	}

	destination[i] = 0;

	return 0;
}

#endif