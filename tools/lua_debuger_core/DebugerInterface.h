
#ifndef _DEBUGER_INTERFACE
#define _DEBUGER_INTERFACE
#define BUFSIZE (32*1024)
typedef bool (*ProcessRetCmd)( char* lpszCommand, size_t size, size_t buffer_size );

enum run_mode { lua_run, lua_debug, lua_stop };

typedef class LuaDebugCommander LuaDebugCommander;

struct _command_buffer
{ 
	char	data[BUFSIZE];
	size_t	size;
	_command_buffer	*next;
};

typedef std::vector< std::string >	Params;
extern bool PraseString( const char* lpszCommand, Params& param );

#define cmd_buffer	1
#define dsp_buffer	2
#define CHECKBUF( B, T, V ) (B?(T==dsp_buffer?( B->data[0] == '$' || B->data[0] == '#' ):( ( B->data[0] == '@' && _strnicmp( V, B->data+1, strlen(V) ) == 0 ) || ( B->data[0] == '#' && (_strnicmp( V, B->data+1, strlen( V ) ) == 0 ) ) ) ):false)

extern LuaDebugCommander* Create_Commander( const char* pipe, ProcessRetCmd fn );
extern _command_buffer* Debug_Command( LuaDebugCommander* Debuger, const char* szFmt, ... );
extern void Debug_ReleaseBuffer( LuaDebugCommander* Debuger, _command_buffer* buf );
extern void Destroy_Commander( LuaDebugCommander* Debuger );
#endif;