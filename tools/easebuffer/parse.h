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
	bool	_immediately;	// �Ƿ�������
	bool	_array;			// �Ƿ�����
	bool	_point;			// �Ƿ�ָ��
	bool	_basic;			// �Ƿ��������
	bool	_container;		// �Ƿ�����
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

	fstream	hfile;		// .h �ļ�
	fstream	cfile;		// .cpp �ļ�
	map< string, list< string > >	mcode;		// ��Ϣ�ṹ
	stack< node * >		pnode;
	list< node* >		snode;		// �ṹ����
	list< message* >	mnode;		// ��Ϣ����
	list< string >		defines;	// Ԥ�����
	list< string >		files;

	struct _config
	{
		string	outdir;		// ����ļ���
		string	prefix;		// Ԥ�����ļ���ǰ׺
		string	userdef;	// �û��Զ������ͷ
		string	yu;			// Ԥ����ͷ�ļ�
	}config;
};
struct command;

char* strntok( char *string, const char *end, const char *control, char **next );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�����﷨��
//--------------------------------------------------------//
void destroyall( root *proot );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	������Ϣ
//--------------------------------------------------------//
void destroymessage( message *pmessage );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	���ٽڵ�
//--------------------------------------------------------//
void destroynode( node *pnode );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	���ٲ���
//--------------------------------------------------------//
void destroyparam( param *pparam );

//--------------------------------------------------------//
//	created:	18:12:2009   16:37
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	����ö�ٶ���
//--------------------------------------------------------//
void destroyenum( enumerate *penum );

//--------------------------------------------------------//
//	created:	14:12:2009   14:51
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�����ļ�
//--------------------------------------------------------//
int analysefile( root *proot, _lpcstr filename );

//--------------------------------------------------------//
//	created:	14:12:2009   15:01
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�����б��еĵ�һ���ַ���
//--------------------------------------------------------//
int findkeywork( char **buf, char *end, command commands[], int count );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	������Ϣ����
//--------------------------------------------------------//
size_t makedefine( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	����ṹ����
//--------------------------------------------------------//
size_t makenode( root *proot, char *buf, size_t size, void* pdata );

//---------------------------------------------------//
// [12/14/2010 Albert]
// Description:	�������Ͻṹ 
//---------------------------------------------------//
size_t makeunion( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	����ö��ֵ
//--------------------------------------------------------//
size_t makeenum( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������
//--------------------------------------------------------//
size_t makeparam( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	����δ֪���Ͳ���
//--------------------------------------------------------//
size_t makeunknowe( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������δ�
//--------------------------------------------------------//
size_t makemodifier( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������
//--------------------------------------------------------//
size_t makebasic( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   18:14
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������
//--------------------------------------------------------//
size_t makecontainer( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   14:51
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������ṹ
//--------------------------------------------------------//
size_t maketree( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   17:23
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	������Ϣ
//--------------------------------------------------------//
size_t makemessage( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	14:12:2009   17:23
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�������ݿ���Ϣ
//--------------------------------------------------------//
size_t makedatabase( root *proot, char *buf, size_t size, void* pdata );

//--------------------------------------------------------//
//	created:	17:12:2009   14:28
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	дһ����Ϣ
//--------------------------------------------------------//
void writemessage( root *proot, message *pmessage );

//--------------------------------------------------------//
//	created:	17:12:2009   14:29
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	дһ���ṹ
//--------------------------------------------------------//
void writenode_def( root *proot, node *pnode, string pix );
void writenode_imp( root *proot, node *pnode, string pix );

//--------------------------------------------------------//
//	created:	18:12:2009   15:24
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	����ͷ�ļ�
//--------------------------------------------------------//
void writeheader( root *proot );

//--------------------------------------------------------//
//	created:	17:12:2009   14:27
//	filename: 	AnalyseFile
//	author:		Albert.xu
//
//	purpose:	�����ɵ���Ϣ��ת����C++�ļ���
//--------------------------------------------------------//
void writefile( root *proot );

#endif // __PARSE_H__