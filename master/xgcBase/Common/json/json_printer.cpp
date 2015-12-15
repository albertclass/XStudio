#include "json_printer.h"
#include "logger.h"
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
		DBG_INFO( "%s%s", node.asCString(), endless );
		break;
	case Json::booleanValue:
		DBG_INFO( "%s%s", node.asBool()?"true":"false", endless );
		break;
	case Json::intValue:
		DBG_INFO( "%d%s", node.asInt(), endless );
		break;
	case Json::uintValue:
		DBG_INFO( "%u%s", node.asUInt(), endless );
		break;
	case Json::realValue:
		DBG_INFO( "%lf%s", node.asDouble(), endless );
		break;
	case Json::arrayValue:
		DBG_INFO("%s", endless);
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			prefix_cur[0] = '|';
			prefix_cur[1] = '-';
			printf( "%s[%d] : ", prefix, i.index() );

			prefix_cur[1] = ' ';
			JsonPrint( *i, prefix, prefix_cur+2, max_deep, endless );
			*prefix_cur = 0;
		}
		break;
	case Json::objectValue:
		DBG_INFO("%s", endless);
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			prefix_cur[0] = '|';
			prefix_cur[1] = '-';
			DBG_INFO( "%s%s : ", prefix, i.key() );

			prefix_cur[1] = ' ';
			JsonPrint( *i, prefix, prefix_cur+2, max_deep, endless );
			*prefix_cur = 0;
		}
		break;
	}
}
