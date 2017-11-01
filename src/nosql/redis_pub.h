#ifndef __REDIS_PUB_H__
#define __REDIS_PUB_H__
class redisConn;

class redisPub
{
private:
	redisConn *conn_;

public:
	explicit redisPub( redisConn* conn )
		: conn_( conn )
	{

	}

};
#endif // __REDIS_PUB_H__