package.path  = package.path  .. ";..\\Scripts\\?.lua"
package.cpath = package.cpath .. ";..\\Scripts\\?.dll"

local stub = require "lua_debuger_stub"
local util = require "lua_debuger_util"

stub.init( 5001, false )
stub.sbrk( "lua_debuger_util.lua", 6 )
stub.wait()

print( "hello world\n" );
local t = {
	"item1",
	"item2",
	"item3",
	
	item1 = 1,
	item2 = 2,
	item3 = 3,

	tbl = {
		item1 = "1",
		item2 = "2",
		item3 = { 1, 3, 5, 7, 9 },
	},

	call = function()
		print( "call" )
	end
}

util.display( t )

stub.fini()