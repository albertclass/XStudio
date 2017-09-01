function do_refresh( ... )
	print( ... )
end

Refresh = {
	{
		start = '2017-08-31 20:00:00',
		params = 'week:1-7',
		invoke = {
			func = do_refresh,
			args = { 1, 2, 3 }
		}
	},

	{
		start = '2017-08-31 20:00:00',
		params = 'week:1-7',
		invoke = {
			func = do_refresh,
			args = { 4, 5, 6 }
		}
	},
}

for k, v in ipairs( Refresh ) do
	print( k, v );
	call = v.invoke
	call.func( unpack( call.args ) )
end

call = Refresh[2].invoke
call.func( unpack( call.args ) )

return unpack( call.args )
