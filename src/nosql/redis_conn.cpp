#include "nosql.h"
#include "redis_conn.h"

redisExecption::redisExecption() throw( )
	: base()
{

}

redisExecption::redisExecption( char const * _Message, int err ) throw( )
	: base( _Message, err )
{

}

redisExecption::redisExecption( char const * const _Message ) throw( )
	: base( _Message )
{

}

redisExecption::redisExecption( redisExecption const & _Other )
	: base( _Other )
{

}

redisExecption& redisExecption::operator=( redisExecption const & _Other )
{
	*static_cast<redisExecption*>( this ) = static_cast<redisExecption const &>( _Other );
	return *this;
}
