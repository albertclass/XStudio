#include "parse.h"

struct command
{
	const char *keyword;
	size_t (*pcommand)( root *proot, char *buf, size_t size, void* pdata );
};

static command typelist[] = 
{
	{ "unsigned",	makemodifier },
	{ "const",		makemodifier },
	{ "bool",		makebasic },
	{ "char",		makebasic },
	{ "wchar_t",	makebasic },
	{ "short",		makebasic },
	{ "int",		makebasic },
	{ "long",		makebasic },
	{ "float",		makebasic },
	{ "double",		makebasic },

	{ "string",		makebasic },
	{ "wstring",	makebasic },
	{ "vector",		makecontainer },
	{ "list",		makecontainer },
	{ "deque",		makecontainer },
	{ "set",		makecontainer },
	{ "map",		makecontainer },
	{ "multimap",	makecontainer },
	{ "multiset",	makecontainer },

	{ "std::string",	makebasic },
	{ "std::wstring",	makebasic },
	{ "std::vector",	makecontainer },
	{ "std::list",		makecontainer },
	{ "std::deque",		makecontainer },
	{ "std::set",		makecontainer },
	{ "std::map",		makecontainer },
	{ "std::multimap",	makecontainer },
	{ "std::multiset",	makecontainer },

};

static command commands[] = 
{
	{ "#define",	makedefine },
	{ "struct",		makenode },
	{ "union",		makeunion },
	{ "message",	makemessage },
	{ "database",	makedatabase },
	{ "enum",		makeenum },
};

char* strntok( char *string, const char *end, const char *control, char **next )
{
	unsigned char *str;
	const unsigned char *ctl = (const unsigned char*)control;
	unsigned char map[32];
	int count;

	if( *next == end ) 
		return NULL;

	/* Clear control map */
	for (count = 0; count < 32; count++)
	{
		map[count] = 0;
	}

	/* Set bits in delimiter table */
	do {
		map[*ctl >> 3] |= (1 << (*ctl & 7));
	} while (*ctl++);

	/* If string is NULL, set str to the saved
	* pointer (i.e., continue breaking tokens out of the string
	* from the last strtok call) */
	if (string != NULL )
	{
		str = (unsigned char*)string;
	}
	else
	{
		str = (unsigned char*)(*next+1);
	}

	/* Find beginning of token (skip over leading delimiters). Note that
	* there is no token iff this loop sets str to point to the terminal
	* null (*str == 0) */
	while ((map[*str >> 3] & (1 << (*str & 7))) && *str != 0 && (char*)str != end )
	{
		str++;
	}

	string = (char*)str;

	/* Find the end of the token. If it is not the end of the string,
	* put a null there. */
	for ( ; *str != 0 ; str++ )
	{
		if ( ( (char*)str == end ) || (map[*str >> 3] & (1 << (*str & 7))) )
		{
			//*str++ = 0;
			break;
		}
	}

	/* Update context */
	*next = (char*)str;

	/* Determine if a token has been found. */
	if (string == (char*)str)
	{
		return NULL;
	}
	else
	{
		return string;
	}
}

void destroyall( root *proot )
{
	list< node * >::iterator inode = proot->snode.begin();
	while( inode != proot->snode.end() )
	{
		destroynode( *inode );
		++inode;
	}
	proot->snode.clear();

	list< message * >::iterator imessage = proot->mnode.begin();
	while( imessage != proot->mnode.end() )
	{
		destroymessage( *imessage );
		++imessage;
	}
	proot->mnode.clear();
}

void destroymessage( message *pmessage )
{
	destroynode( &pmessage->sub );
}

void destroynode( node *pnode )
{
	list< enumerate * >::iterator ienum = pnode->enums.begin();
	while( ienum != pnode->enums.end() )
	{
		destroyenum( *ienum );
		++ienum;
	}
	pnode->enums.clear();

	list< param * >::iterator iparam = pnode->params.begin();
	while( iparam != pnode->params.end() )
	{
		destroyparam( *iparam );
		++iparam;
	}
	pnode->params.clear();

	list< node * >::iterator inode = pnode->sub.begin();
	while( inode != pnode->sub.end() )
	{
		destroynode( *inode );
		++inode;
	}
	pnode->sub.clear();
	delete pnode;
}

void destroyparam( param *pparam )
{
	delete pparam;
}

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁枚举对象
//--------------------------------------------------------//
void destroyenum( enumerate *penum )
{
	penum->items.clear();
	delete penum;
}

int analysefile( root *proot, _lpcstr filename )
{
	int ret = 0;
	FILE *fp;
	fopen_s( &fp, filename, "rb" );
	if( fp )
	{
		printf( "analyse file %s...\n", filename );
		fseek( fp, 0, SEEK_END );
		size_t size = proot->size = ftell(fp);

		fseek( fp, 0, SEEK_SET );
		char *buf = proot->buf = (char*)malloc( size );
		char *bend = buf + size;

		if( fread( buf, size, 1, fp ) != 1 && !feof(fp) )
		{
			fclose( fp );
			return -1;
		}

		string fname = filename;
		proot->filename = fname.substr( fname.rfind( '\\' )+1, fname.rfind( '.' ) - fname.rfind( '\\' ) - 1 );
		proot->files.push_back( proot->filename );
		for( ;; )
		{
			int idx = findkeywork( &buf, bend, commands, _countof(commands) );
			if( idx != -1 )
			{
				size_t read = commands[idx].pcommand( proot, buf, bend - buf, NULL );
				if( read == -1 )
				{
					++ret;
					++buf;
					--size;
				}
				buf += read;
			}
			else
			{
				break;
			}
		}
		proot->filename.clear();
		fclose( fp );
	}
	else
	{
		printf( "file %s open failed.\n", filename );
	}
	return ret;
}

//--------------------------------------------------------//
//	created:	14:12:2009   17:29
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	获取行号
//--------------------------------------------------------//
int countline( char *buf, char* cur )
{
	int n = 1;
	while( buf < cur )
	{
		while( buf < cur && *buf++ != '\n' );
		++n;
	}
	return n;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:29
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	匹配中括号
//--------------------------------------------------------//
char* matchclose( char *buf, size_t size, char lch = '{', char rch = '}' )
{
	if( *buf != lch )
		return NULL;

	int n = 0;
	char *cur = buf;
	while( cur < buf + size )
	{
		if( (*cur == lch?++n:*cur == rch?--n:n) == 0 )
		{
			return strntok( NULL, buf+size, " }\r\n\t", &cur )?cur:NULL;
		}
		++cur;
	}
	return NULL;
}

//--------------------------------------------------------//
//	created:	14:12:2009   15:01
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	查找列表中的第一个字符串
//--------------------------------------------------------//
int findkeywork( char **buf, char *end, command commands[], int count )
{
	while( *buf < end )
	{
		for( int i = 0; i < count; ++i )
		{
			int len = strlen(commands[i].keyword);
			if( strncmp( *buf, commands[i].keyword, len ) == 0 )
			{
				*buf += len;
				return i;
			}
		}
		++*buf;
	}
	return -1;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造消息定义
//--------------------------------------------------------//
size_t makedefine( root *proot, char *buf, size_t size, void* pdata )
{
	char *begin = buf;
	char *end = buf;
	do
	{
		strntok( begin, buf + size, "\r\n", &end );
		if( strntok( begin, end, "\\", &begin ) && begin != end )
		{
			strntok( begin, end, " \t", &begin );
		}
		else
		{
			break;
		}
	}while( begin && ++begin == ++end );

	string def(buf, end);
	printf( "\tmake #define %s\n", def.c_str() );
	proot->defines.push_back( "#define" + def );
	return end - buf;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造结构定义
//--------------------------------------------------------//
size_t makenode( root *proot, char *buf, size_t size, void* pdata )
{
	char *next = NULL;
	char *name = strntok( buf, buf+size, " {\t\r\n", &next );
	char *namend = next;
	char *begin = strntok( NULL, buf+size, " \t\r\n", &next );

	if( name && begin && *begin == '{' )
	{
		node *pnode = new node;
		pnode->name.append( name, namend );
		proot->pnode.push( pnode );
		size_t read = maketree( proot, begin, size - ( begin - buf ), NULL );
		if( read == -1 )
		{
			destroynode( pnode );
			printf( "cannot found match '}' at %d", countline( proot->buf, buf ) );
			return -1;
		}
		else
		{
			proot->snode.push_back( pnode );
			return next - buf + read;
		}
	}
	else
	{
		printf( "struct format error at %d, use :\nstruct STRUCTNAME\n{\n\tSOMEBODY;\n};\n", countline( proot->buf, buf ) );
		return -1;
	}
}

//---------------------------------------------------//
// [12/14/2010 Albert]
// Description:	构造联合结构 
//---------------------------------------------------//
size_t makeunion( root *proot, char *buf, size_t size, void* pdata )
{
	return -1;
}

//--------------------------------------------------------//
//	created:	15:12:2009   17:39
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	是否是列表中的串
//--------------------------------------------------------//
int iskeywork( root *proot, char *buf, size_t size, command commands[], int count )
{
	char *next = NULL;
	char *token = strntok( buf, buf+size, " ;()<>[]{}\t\r\n", &next );
	if( token )
	{
		for( int i = 0; i < count; ++i )
		{
			if( strncmp( token, commands[i].keyword, strlen(commands[i].keyword) ) == 0 )
			{
				return i;
			}
		}
	}

	return -1;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makeparam( root *proot, char *buf, size_t size, void* pdata )
{
	char seps[] = " <>\t\r\n";
	char *next = NULL;
	if( pdata == NULL )
	{
		node *pnode = proot->pnode.top();
		param *param_ = new param;
		param_->tline.append( strntok( buf, buf+size, seps, &next ), buf+size );
		param_->_immediately = false;
		param_->_array = false;
		param_->_basic = false;
		param_->_point = false;
		param_->_container = false;

		pnode->params.push_back( param_ );
		pdata = (void*)param_;
	}

	int ret = iskeywork( proot, buf, size, typelist, _countof(typelist) );
	if( ret != -1 )
	{
		return typelist[ret].pcommand( proot, buf, size, pdata );
	}
	else
	{
		return makeunknowe( proot, buf, size, pdata );
	}
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造未知类型参数
//--------------------------------------------------------//
size_t makeunknowe( root *proot, char *buf, size_t size, void* pdata )
{
	param* pparam = (param*)pdata;
	char *next = NULL;
	char *token = strntok( buf, buf+size, " <>\t\r\n", &next );
	if( *next == '<' )
	{
		token = matchclose( next, size - (buf - next), '<', '>' );
		if( !next )
			return -1;

		pparam->_container = true;
	}
	else
	{
		pparam->_basic = true;
	}

	pparam->tname.append( token, next );
	printf( "make unknowe type %s at line %d \n", pparam->tname.c_str(), countline( proot->buf, buf ) );

	token = strntok( NULL, buf+size, " ;[]\t\r\n", &next );
	if( token )
	{
		if( *next == '[' )
		{
			char *c = matchclose( next, size - ( buf - next ), '[', ']' );
			if( !c )
				return -1;
			pparam->_array = true;
		}
		pparam->pname.append( token, next );
	}
	return size;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造修饰词
//--------------------------------------------------------//
size_t makemodifier( root *proot, char *buf, size_t size, void* pdata )
{
	char *next = NULL;
	char *token = strntok( buf, buf+size, " ;()<>[]{}\t\r\n", &next );
	param *param_ = (param*)pdata;
	if( param_ )
	{
		param_->tname.append( token, next+1 );
	}

	size_t ret = makeparam( proot, next, buf + size - next, pdata );
	if( ret == -1 )
	{
		printf( "make param error at line %d", countline( proot->buf, buf ) );
	}
	return size;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makebasic( root *proot, char *buf, size_t size, void* pdata )
{
	param *param_ = (param*)pdata;
	param_->_basic = true;
	vector< string > part;
	int n = 0, nary = 0;

	for( char *cur = buf, *beg = buf; cur != buf+size; ++cur )
	{
		switch( *cur )
		{
		case '[':
			if( cur != beg )
			{
				part.push_back( string( beg, cur - beg ) );
			}
			beg = cur+1;
			break;
		case ']':
			param_->_array = true;
			part.push_back( string( beg, cur - beg ) );
			beg = cur+1;
			++nary;
			break;
		case ' ':
		case ';':
		case '\n':
		case '\r':
		case '\t':
			if( cur != beg ) 
				part.push_back( string( beg, cur - beg ) );
			beg = cur+1;
			break;
		}
	}

	if( part.size() != (param_->_array?3:2) )
		return -1;

	param_->tname += part[0];
	param_->pname = part[1];
	return size;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makecontainer( root *proot, char *buf, size_t size, void* pdata )
{
	param *param_ = (param*)pdata;
	vector< string > part;
	int n = 0, ntpl = 0, nary = 0;

	for( char *cur = buf, *beg = buf; cur != buf+size; ++cur )
	{
		switch( *cur )
		{
		case '<':
			if( ntpl == 0 )
			{
				part.push_back( string( beg, cur - beg ) );
				beg = cur+1;
			}
			++ntpl;
			break;
		case '>':
			--ntpl;
			if( ntpl == 0 )
			{
				param_->_container = true;
				part.push_back( string( beg, cur - beg ) );
				beg = cur+1;
			}
			break;
		case '[':
			if( cur != beg )
			{
				part.push_back( string( beg, cur - beg ) );
			}
			beg = cur+1;
			++nary;
			break;
		case ']':
			param_->_array = true;
			part.push_back( string( beg, cur - beg ) );
			beg = cur+1;
			--nary;
			break;
		case ' ':
		case ';':
		case '\n':
		case '\r':
		case '\t':
			if( ntpl == 0 && nary == 0 )
			{
				if( cur != beg ) 
					part.push_back( string( beg, cur - beg ) );
				beg = cur+1;
			}
			break;
		}
	}
	param_->tname += part[0] + "< " + part[1] + " >";
	param_->pname = part[2];
	return size;
}

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造枚举值
//--------------------------------------------------------//
size_t makeenum( root *proot, char *buf, size_t size, void* pdata )
{
	char *match = matchclose( buf, size );
	if( !match ) return -1;

	enumerate *penum = new enumerate;
	penum->name = (char*)pdata;
	char *next = NULL;
	char *token = strntok( buf, match, " ,{};\t\r\n", &next );
	while( token )
	{
		if( *token == '/' && *(token+1) == '/' )
		{
			next = strchr( token, '\n' );
			if( next > match )
				break;
		}
		else
		{
			penum->items.push_back( string( token, next ) );
		}
		
		token = strntok( NULL, match, " ,{};\t\r\n", &next );
	}
	node* pnode = proot->pnode.top();
	pnode->enums.push_back( penum );

	return match - buf;
}

//--------------------------------------------------------//
//	created:	14:12:2009   14:51
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造树结构
//--------------------------------------------------------//
size_t maketree( root *proot, char *buf, size_t size, void* pdata )
{
	char *match = matchclose( buf, size );
	if( !match ) return -1;

	char *bseg = buf+1, *cur = buf+1;
	size_t read = 0;
	size = match - buf;

	do
	{
		switch( *cur )
		{
		case ';':
			read = makeparam( proot, bseg, cur + 1 - bseg, NULL );
			if( read == -1 )
			{
				printf( "param format error at line %d\n", countline( proot->buf, bseg ) );
			}
			bseg = cur + 1;
			break;
		case '/':
			if( *(cur+1) == '/' )
			{
				strntok( cur, match, "\n\r", &bseg );
				param *mtype  = new param;
				mtype->_array = false;
				mtype->_basic = false;
				mtype->_container = false;
				mtype->_point = false;
				mtype->_immediately = false;
				mtype->tline.append( cur, bseg );
				node *pnode = proot->pnode.top();
				pnode->params.push_back( mtype );

				cur = bseg;
			}
			break;
		case '{':
			{
				char *next = NULL, seps[] = " {};()\n\r\t";
				char *token = strntok( bseg, cur, seps, &next );
				char *part[256][2];
				int i = 0;

				memset( part, 0, sizeof(part) );
				for( ; token && i < _countof(part); ++i )
				{
					part[i][0] = token;
					part[i][1] = next;
					token = strntok( NULL, cur, seps, &next );
				}

				if( i == 0 )
				{
					printf( "cannot found keyword at %d", countline( proot->buf, cur ) );
					bseg = cur = match;
					break;
				}

				string key( part[0][0], part[0][1] );
				if( key == "struct" )
				{
					if( i != 2 )
					{
						printf( "struct format error at %d", countline( proot->buf, cur ) );
						bseg = cur = match;
						break;
					}
					node *sub = new node;
					node *pnode = proot->pnode.top();
					// 命名空间
					sub->spacename = pnode->spacename + pnode->name + "::";
					sub->name.append( part[1][0], part[1][1] );

					proot->pnode.push( sub );
					size_t read = maketree( proot, cur, size - ( cur - buf ), NULL );
					if( read == -1 )
					{
						destroynode( sub );
						printf( "cannot found match '}' at %d", countline( proot->buf, cur ) );
						bseg = cur = match;
					}
					else
					{
						pnode->sub.push_back( sub );
						cur += read;
						bseg = cur;
					}
				}
				else if( key == "enum" )
				{
					string name( part[1][0], part[1][1] );
					read = makeenum( proot, cur, size - ( cur - buf ), (void*)name.c_str() );
					if( read == -1 )
					{
						printf( "param format error at line %d\n", countline( proot->buf, bseg ) );
					}
					cur = bseg = cur + read;
				}
				else
				{
					printf( "struct format error at %d, use :\nstruct STRUCTNAME\n{\n\tSOMEBODY;\n};\n", countline( proot->buf, bseg ) );
					proot->pnode.pop();
					return match - buf;
				}
			}
			break;
		case '}':
			proot->pnode.pop();
			return size;
			break;
		}
	}while( ++cur < match );

	proot->pnode.pop();
	return -1;
}

size_t makemessage( root *proot, char *buf, size_t size, void* pdata )
{
	message *msg = new message;

	msg->filename = proot->filename;
	char seps[] = " ,()\n\r\t";
	char *part[3][2];
	char *next = NULL, *end = strchr( buf, '{' );
	size_t total = 0;
	if( end )
	{
		int i = 0;
		char *token = strntok( buf, end, seps, &next );
		for( ; token && i < _countof(part); token = strntok( NULL, end, seps, &next ), ++i )
		{
			part[i][0] = token;
			part[i][1] = next;
		}

		if( i == _countof(part) )
		{
			msg->sub.name.append( part[0][0], part[0][1] );
			msg->stype.append( part[1][0], part[1][1] );
			msg->scode.append( part[2][0], part[2][1] );

			// 添加两个立即数到参数中
			param *mtype  = new param;
			mtype->_array = false;
			mtype->_basic = true;
			mtype->_container = false;
			mtype->_point = false;
			mtype->_immediately = true;
			mtype->tline = "_byte mtype;";
			mtype->tname = "_byte";
			mtype->pname = "mtype";
			mtype->pvalue = msg->stype;
			msg->sub.params.push_back( mtype );

			param *mcode  = new param;
			mcode->_array = false;
			mcode->_basic = true;
			mcode->_container = false;
			mcode->_point = false;
			mcode->_immediately = true;
			mcode->tline = "_byte mcode;";
			mcode->tname = "_byte";
			mcode->pname = "mcode";
			mcode->pvalue = msg->scode;
			msg->sub.params.push_back( mcode );

			//param *vcode  = new param;
			//vcode->_array = false;
			//vcode->_basic = true;
			//vcode->_container = false;
			//vcode->_point = false;
			//vcode->_immediately = true;
			//vcode->tline = "_uint16 vcode;";
			//vcode->tname = "_uint16";
			//vcode->pname = "vcode";
			//vcode->pvalue = "0";
			//msg->sub.params.push_back( vcode );

			proot->mcode[msg->stype].push_back( msg->scode );
		}
		else
		{
			printf( "message format error at %d, use : \nmessage MESSAGENAME( TYPE, CODE )\n{ \nSOMEBODY;\n };\n", countline( proot->buf, buf ) );
			return false;
		}
		proot->pnode.push( &msg->sub );
		printf( "\tmake message %s\n", msg->sub.name.c_str() );
		size_t read = maketree( proot, end, size - ( end - buf ), NULL );
		if( read == -1 )
		{
			printf( "cannot found match '}' at %d\n", countline( proot->buf, buf ) );
			return -1;
		}
		total += end - buf + read;
	}
	proot->mnode.push_back( msg );
	return total;
}

//--------------------------------------------------------//
//	created:	14:12:2009   17:23
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造数据库消息
//--------------------------------------------------------//
size_t makedatabase( root *proot, char *buf, size_t size, void* pdata )
{
	message *msg = new message;

	msg->filename = proot->filename;
	char seps[] = " ,()\n\r\t";
	char *part[3][2];
	char *next = NULL, *end = strchr( buf, '{' );
	size_t total = 0;
	if( end )
	{
		int i = 0;
		char *token = strntok( buf, end, seps, &next );
		for( ; token && i < _countof(part); token = strntok( NULL, end, seps, &next ), ++i )
		{
			part[i][0] = token;
			part[i][1] = next;
		}

		if( i == _countof(part) )
		{
			msg->sub.name.append( part[0][0], part[0][1] );
			msg->stype.append( part[1][0], part[1][1] );
			msg->scode.append( part[2][0], part[2][1] );

			// 添加两个立即数到参数中
			param *mtype  = new param;
			mtype->_array = false;
			mtype->_basic = true;
			mtype->_container = false;
			mtype->_point = false;
			mtype->_immediately = true;
			mtype->tline = "_byte mtype;";
			mtype->tname = "_byte";
			mtype->pname = "mtype";
			mtype->pvalue = msg->stype;
			msg->sub.params.push_back( mtype );

			param *mcode  = new param;
			mcode->_array = false;
			mcode->_basic = true;
			mcode->_container = false;
			mcode->_point = false;
			mcode->_immediately = true;
			mcode->tline = "_byte mcode;";
			mcode->tname = "_byte";
			mcode->pname = "mcode";
			mcode->pvalue = msg->scode;
			msg->sub.params.push_back( mcode );

			param *uid  = new param;
			uid->_array = false;
			uid->_basic = true;
			uid->_container = false;
			uid->_point = false;
			uid->_immediately = false;
			uid->tline = "short uid;";
			uid->tname = "short";
			uid->pname = "uid";
			msg->sub.params.push_back( uid );
		}
		else
		{
			printf( "message format error at %d, use : \nmessage MESSAGENAME( TYPE, CODE )\n{ \nSOMEBODY;\n };\n", countline( proot->buf, buf ) );
			return false;
		}
		proot->pnode.push( &msg->sub );
		printf( "\tmake message %s\n", msg->sub.name.c_str() );
		size_t read = maketree( proot, end, size - ( end - buf ), NULL );
		if( read == -1 )
		{
			printf( "cannot found match '}' at %d\n", countline( proot->buf, buf ) );
			return -1;
		}
		total += end - buf + read;
	}
	proot->mnode.push_back( msg );
	return total;
}
//--------------------------------------------------------//
//	created:	17:12:2009   14:28
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	写一个消息
//--------------------------------------------------------//
void writemessage( root *proot, message *pmessage )
{
	if( proot->filename != pmessage->filename )
	{
		proot->hfile << "#endif //" << proot->filename << "_MESSAGE_H_" << endl;
		proot->hfile.close();
		proot->hfile.clear();
		proot->cfile.close();
		proot->cfile.clear();

		proot->filename = pmessage->filename;
		string hfile = proot->config.outdir + "\\" + proot->filename+".h";
		string cfile = proot->config.outdir + "\\" + proot->filename+".cpp";
		proot->hfile.open( hfile.c_str(), ios_base::out|ios_base::trunc );
		if( !proot->hfile.is_open() )
		{
			printf("open %s failed.", hfile.c_str() );
			return;
		}
		else
		{
			proot->hfile << "#include \"" << proot->config.prefix << "messagedef.h\"" << endl;
			proot->hfile << "#ifndef " << proot->filename << "_MESSAGE_H_" << endl;
			proot->hfile << "#define " << proot->filename << "_MESSAGE_H_" << endl;

			if( proot->snode.size() )
			{
				proot->hfile << "#include \"" << proot->config.prefix << "structsdef.h\"" << endl;
			}
		}
		proot->cfile.open( cfile.c_str(), ios_base::out|ios_base::trunc );
		if( !proot->cfile.is_open() )
		{
			printf("open %s failed.", cfile.c_str() );
			return;
		}
		else
		{
			if( !proot->config.yu.empty() )
			{
				proot->cfile << "#include \"" << proot->config.yu << "\"" << endl;
			}
			
			proot->cfile << "#include \"" << proot->filename << ".h\"" << endl;
			proot->hfile
				<< "#ifdef _USRDLL" << endl
				<< "\t#ifdef MESSAGE_API" << endl
				<< "\t\t#define MESSAGE_EXPORT __declspec(dllexport)" << endl
				<< "\t#else" << endl
				<< "\t\t#define MESSAGE_EXPORT __declspec(dllimport)" << endl 
				<< "\t#endif" << endl
				<< "#else" << endl
				<< "\t#define MESSAGE_EXPORT" << endl
				<< "#endif" << endl
				<< endl;
		}
	}
	writenode_def( proot, &pmessage->sub, "" );
	writenode_imp( proot, &pmessage->sub, "" );
}

//--------------------------------------------------------//
//	created:	17:12:2009   14:29
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	写一个结构
//--------------------------------------------------------//
void writenode_def( root *proot, node *pnode, string pix )
{
	fstream &hstream = proot->hfile;
	hstream << pix << "struct " << "MESSAGE_EXPORT " << pnode->name << endl
		<< pix << "{" << endl;

	list< enumerate* >::iterator ienum = pnode->enums.begin();
	while( ienum != pnode->enums.end() )
	{
		enumerate *penum = *ienum;
		bool singleline = penum->items.size() < 6;
		hstream << pix << "\t" << "enum " << penum->name;
		if( !singleline ) hstream << endl << pix << "\t" ;
		hstream << "{ ";
		if( !singleline ) hstream << endl << pix << "\t\t" ;

		list< string >::iterator iitem = penum->items.begin();
		while( iitem != penum->items.end() )
		{
			hstream << *iitem << ", ";
			if( !singleline ) hstream << endl << pix << "\t\t";
			++iitem;
		}

		hstream << "};" << endl;
		++ienum;
	}

	list< node* >::iterator inode = pnode->sub.begin();
	while( inode != pnode->sub.end() )
	{
		writenode_def( proot, *inode, pix+"\t" );
		++inode;
	}

	stringstream opi_stream, opo_stream;
	opi_stream << endl 
		<< "MESSAGE_EXPORT bufstream& operator << ( bufstream& stream, const " << pnode->spacename + pnode->name << "& c )" << endl
		<< "{" << endl;

	opo_stream << endl
		<< "MESSAGE_EXPORT bufstream& operator >> ( bufstream& stream, " << pnode->spacename + pnode->name << "& c )" << endl
		<< "{" << endl;

	list< param* >::iterator iparam = pnode->params.begin();
	while( iparam != pnode->params.end() )
	{
		param* pparam = *iparam;

		// write param define.
		if( !pparam->_immediately )
			hstream << pix << "\t" << pparam->tline << endl;

		if( pparam->_immediately )
		{
			opi_stream << "\tstream << " << pparam->tname << "(" << pparam->pvalue << ");" << endl;
			//opo_stream << "\tstream >> c." << pparam->pname << ";" << endl;
		}
		else if( pparam->_array && pparam->_basic)
		{
			opi_stream << "\tstream << cbuffer( (char*)c." << pparam->pname << ", sizeof(c." << pparam->pname << ") );" << endl;
			opo_stream << "\tstream >> cbuffer( (char*)c." << pparam->pname << ", sizeof(c." << pparam->pname << ") );" << endl;
		}
		else if( pparam->_array )
		{
			opi_stream << "\tfor( int i = 0; i < _countof(c." << pparam->pname << "); ++i )" << endl
				<< "\t\tstream << c." << pparam->pname << "[i];" << endl;

			opo_stream << "\tfor( int i = 0; i < _countof(c." << pparam->pname << "); ++i )" << endl
				<< "\t\tstream >> c." << pparam->pname << "[i];" << endl;
		}
		else if( pparam->_basic || pparam->_container )
		{
			opi_stream << "\tstream << c." << pparam->pname << ";" << endl;
			opo_stream << "\tstream >> c." << pparam->pname << ";" << endl;
		}
		++iparam;
	}
	opi_stream << "\treturn stream;" << endl << "}" << endl;
	opo_stream << "\treturn stream;" << endl << "}" << endl;
	proot->cfile << opi_stream.rdbuf();
	proot->cfile << opo_stream.rdbuf();

	hstream << pix << "};" << endl;
}

void writenode_imp( root *proot, node *pnode, string pix )
{
	list< node* >::iterator inode = pnode->sub.begin();
	while( inode != pnode->sub.end() )
	{
		writenode_imp( proot, *inode, "" );
		++inode;
	}

	fstream &hstream = proot->hfile;
	hstream << endl;
	hstream << "MESSAGE_EXPORT bufstream& operator << ( bufstream& stream, const " << pnode->spacename + pnode->name << "& c );" << endl;
	hstream	<< "MESSAGE_EXPORT bufstream& operator >> ( bufstream& stream, " << pnode->spacename + pnode->name << "& c );" << endl;
	hstream << endl << "/////////////////////////////////////////////////////////////////////////////////" << endl;
}

void writeheader( root *proot )
{
	string hfile = proot->config.outdir + "\\" + proot->config.prefix + "messagedef.h";
	fstream stream;
	stream.open( hfile.c_str(), ios_base::out|ios_base::trunc );
	if( !stream.is_open() )
	{
		printf("open messagedef.h failed. filepath = %s", hfile.c_str() );
		return;
	}
	else
	{
		stream << "#ifndef " << proot->config.prefix << "_MESSAGEDEF_H" << endl;
		stream << "#define " << proot->config.prefix << "_MESSAGEDEF_H" << endl;
		stream << "#include \"commonlib.h\"" << endl;
		if( !proot->config.userdef.empty() )
		{
			stream << "#include \"" << proot->config.userdef << "\"" << endl;
		}
		stream << "using namespace std;" << endl;
		stream << "using namespace XGC::common;" << endl;

		list< string >::iterator iter = proot->defines.begin();
		while( iter != proot->defines.end() )
		{
			stream << *iter << endl;
			++iter;
		}

		map< string, list< string > >::iterator itypemap = proot->mcode.begin();
		while( itypemap != proot->mcode.end() )
		{
			stream << endl << "enum EN_" << itypemap->first << endl << "{" << endl;
			list< string >::iterator icode = itypemap->second.begin();
			while( icode != itypemap->second.end() )
			{
				stream << "\t" << *icode << "," << endl;
				++icode;
			}
			stream << "\t" << itypemap->first << "_COUNT" << endl;
			stream << "};" << endl;
			++itypemap;
		}
		stream << "#endif //_MESSAGEDEF_H" << endl;
		stream.close();
	}
}

//--------------------------------------------------------//
//	created:	17:12:2009   14:27
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	将生成的消息树转换成C++文件。
//--------------------------------------------------------//
void writefile( root *proot )
{
	if( proot->snode.size() )
	{
		string hfile = proot->config.outdir + "\\" + proot->config.prefix + "structsdef.h";
		string cfile = proot->config.outdir + "\\" + proot->config.prefix + "structsdef.cpp";
		proot->hfile.open( hfile.c_str(), ios_base::out|ios_base::trunc );
		proot->cfile.open( cfile.c_str(), ios_base::out|ios_base::trunc );

		proot->hfile << "#include \"" << proot->config.prefix << "messagedef.h\"" << endl;
		proot->hfile << "#ifndef " << proot->config.prefix << "_STRUCTSDEF_H_" << endl;
		proot->hfile << "#define " << proot->config.prefix << "_STRUCTSDEF_H_" << endl;

		proot->cfile << "#include \"" << proot->config.prefix << "structsdef.h\"" << endl;
		proot->hfile
			<< "#ifdef _USRDLL" << endl
			<< "\t#ifdef MESSAGE_API" << endl
			<< "\t\t#define MESSAGE_EXPORT __declspec(dllexport)" << endl
			<< "\t#else" << endl
			<< "\t\t#define MESSAGE_EXPORT __declspec(dllimport)" << endl 
			<< "\t#endif" << endl
			<< "#else" << endl
			<< "\t#define MESSAGE_EXPORT" << endl
			<< "#endif" << endl
			<< endl;

		list< node * >::iterator inode = proot->snode.begin();
		while( inode != proot->snode.end() )
		{
			writenode_def( proot, *inode, "" );
			writenode_imp( proot, *inode, "" );
			++inode;
		}
		proot->hfile << "#endif //_STRUCTSDEF_H_" << endl;
		proot->hfile.close();
		proot->cfile.close();

		proot->files.push_back( proot->config.prefix + "structsdef" );
	}

	list< message * >::iterator imessage = proot->mnode.begin();
	while( imessage != proot->mnode.end() )
	{
		writemessage( proot, *imessage );
		++imessage;
	}

	proot->hfile << "#endif //" << proot->filename << "_MESSAGE_H_" << endl;
	proot->hfile.close();
	proot->cfile.close();

	writeheader( proot );
}