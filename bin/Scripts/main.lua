require "lua_debuger_stub"

debuger.init( 5001, false )
debuger.sbrk( "main.lua", 6 )
debuger.wait()

print( "hello world\n" );

debuger.fini()