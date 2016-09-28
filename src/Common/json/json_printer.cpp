#include "json_printer.h"
//////////////////////////////////////////////////////////////////////////
//
// 打印JSON对象内容
// 示例：
// 	char prefix[256] = { ' ' };
//	JsonPrint( json_object, prefix, prefix+1, _countof(prefix), "\r\n" );
//////////////////////////////////////////////////////////////////////////
void JsonPrint( const Json::Value& node, char *prefix, char *prefix_cur, int max_deep, char *endless )
{
	if( prefix_cur - prefix >= max_deep ) return;

	switch( node.type() )
	{
	case Json::stringValue:
		printf( "%s%s", node.asCString(), endless );
		break;
	case Json::booleanValue:
		printf( "%s%s", node.asBool() ? "true" : "false", endless );
		break;
	case Json::intValue:
		printf( "%d%s", node.asInt(), endless );
		break;
	case Json::uintValue:
		printf( "%u%s", node.asUInt(), endless );
		break;
	case Json::realValue:
		printf( "%lf%s", node.asDouble(), endless );
		break;
	case Json::arrayValue:
		printf( "%s", endless );
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			prefix_cur[0] = '|';
			prefix_cur[1] = '-';
			printf( "%s[%d] : ", prefix, i.index() );

			prefix_cur[1] = ' ';
			JsonPrint( *i, prefix, prefix_cur + 2, max_deep, endless );
			*prefix_cur = 0;
		}
		break;
	case Json::objectValue:
		printf( "%s", endless );
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			prefix_cur[0] = '|';
			prefix_cur[1] = '-';
			printf( "%s%s : ", prefix, i.key().asCString() );

			prefix_cur[1] = ' ';
			JsonPrint( *i, prefix, prefix_cur + 2, max_deep, endless );
			*prefix_cur = 0;
		}
		break;
	}
}
