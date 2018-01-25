local write = io.write

local util = {}

util.display_var = function( var, perfix )
	local perfix = perfix or ""
	local t = type(var)

	if t == "nil" then
		write( "nil" )
	elseif t == "boolean" then
		write( tostring(var) )
	elseif t == "number" then
		write( tostring(var) )
	elseif t == "string" then
		write( '"' .. var .. '"' )
	elseif t == "function" then
		write( "(" .. tostring(var) .. ")" )
	elseif t == "userdata" then
		write( "userdata(" .. string.format( "%x", tonumber(var) ) .. ")" )
	elseif t == "thread" then
		write( "thread" )
	elseif t == "table" then
		write( "{\n" )
		for k, v in pairs( var ) do
			write( perfix .. '\t[' )
			util.display_var( k, perfix )
			write( "] = " )
			util.display_var( v, perfix .. "\t" )
			write( ",\n" )
		end
		write( perfix .. "}" )
	end
end

util.display = function( var, perfix )
	util.display_var( var, perfix )
	write( "\n" )
end

return util;
-- util.display( util )