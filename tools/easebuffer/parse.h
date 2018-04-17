#pragma once
#ifndef __PARSE_H__
#define __PARSE_H__

#include <map>
#include <list>
#include <stack>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

typedef const char*			_lpcstr;
typedef char*				_lpstr;

struct param
{
	string	tline;
	string	comment;
	string	tname;
	string	pname;
	string	pvalue;
	bool	_immediately;	// 是否立即数
	bool	_array;			// 是否数组
	bool	_point;			// 是否指针
	bool	_basic;			// 是否基本类型
	bool	_container;		// 是否容器
};

struct enumerate
{
	string name;
	list< string >	items;
};

struct node
{
	string	name;
	string	spacename;

	list< node* >	sub;
	list< param* >	params;

	list< enumerate* >	enums;
};

struct message
{
	node sub;
	string stype;
	string scode;
	string filename;
};

struct root
{
	size_t	size;
	char	*buf;
	string	filename;

	fstream	hfile;		// .h 文件
	fstream	cfile;		// .cpp 文件
	map< string, list< string > >	mcode;		// 消息结构
	stack< node * >		pnode;
	list< node* >		snode;		// 结构定义
	list< message* >	mnode;		// 消息定义
	list< string >		defines;	// 预定义宏
	list< string >		files;

	struct _config
	{
		string	outdir;		// 输出文件名
		string	prefix;		// 预定义文件名前缀
		string	userdef;	// 用户自定义包含头
		string	yu;			// 预编译头文件
	}config;
};
struct command;

char* strntok( char *string, const char *end, const char *control, char **next );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁语法树
//--------------------------------------------------------//
void destroyall( root *proot );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁消息
//--------------------------------------------------------//
void destroymessage( message *pmessage );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁节点
//--------------------------------------------------------//
void destroynode( node *pnode );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁参数
//--------------------------------------------------------//
void destroyparam( param *pparam );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	销毁枚举对象
//--------------------------------------------------------//
void destroyenum( enumerate *penum );

//--------------------------------------------------------//
//	created:	14:12:2009   14:51
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	分析文件
//--------------------------------------------------------//
int analysefile( root *proot, _lpcstr filename );

//--------------------------------------------------------//
//	created:	14:12:2009   15:01
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	查找列表中的第一个字符串
//--------------------------------------------------------//
int findkeywork( char **buf, char *end, command commands[], int count );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造消息定义
//--------------------------------------------------------//
size_t makedefine( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造结构定义
//--------------------------------------------------------//
size_t makenode( root *proot, char *buf, size_t size, void* pdata );

//---------------------------------------------------//
// [12/14/2010 Albert]
// Description:	构造联合结构 
//---------------------------------------------------//
size_t makeunion( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造枚举值
//--------------------------------------------------------//
size_t makeenum( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makeparam( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造未知类型参数
//--------------------------------------------------------//
size_t makeunknowe( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造修饰词
//--------------------------------------------------------//
size_t makemodifier( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makebasic( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造参数
//--------------------------------------------------------//
size_t makecontainer( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   14:51
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造树结构
//--------------------------------------------------------//
size_t maketree( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   17:23
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造消息
//--------------------------------------------------------//
size_t makemessage( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   17:23
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	构造数据库消息
//--------------------------------------------------------//
size_t makedatabase( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	17:12:2009   14:28
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	写一个消息
//--------------------------------------------------------//
void writemessage( root *proot, message *pmessage );

//--------------------------------------------------------//
//	created:	17:12:2009   14:29
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	写一个结构
//--------------------------------------------------------//
void writenode_def( root *proot, node *pnode, string pix );
void writenode_imp( root *proot, node *pnode, string pix );

//--------------------------------------------------------//
//	created:	18:12:2009   15:24
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	生成头文件
//--------------------------------------------------------//
void writeheader( root *proot );

//--------------------------------------------------------//
//	created:	17:12:2009   14:27
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	将生成的消息树转换成C++文件。
//--------------------------------------------------------//
void writefile( root *proot );

#endif // __PARSE_H__