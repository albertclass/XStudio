from Module import Session

def call( code ):
	def invoke( **kw ):
		_locals = locals()
		for key, value in kw.items():
			_locals[key] = value

		return eval(code)

	return invoke

if __name__ == '__main__':
	print( "%s : %d" % ( Session.name(Session.chat.CHAT_USERAUTH_REQ, Session.chat.CHAT_MSG_ID), Session.chat.CHAT_USERAUTH_REQ ) )
	msg = Session.make( Session.chat.CHAT_USERAUTH_REQ )
	des = msg.DESCRIPTOR
	print(type(des))
	msg.token = 'albert'

	for (name, field) in des.fields_by_name.items():
		print( "%s:%s" % (name, field) )
		print( getattr(msg, name) )
		
	cp = call( 'print( "%s:%d" % (addr,port) )' )
	cp( addr='127.0.0.1', port=2555)

	def throw_str( d ):
		return d['str'] or 'sss'

	d = {
		'1' : '111'
	}

	print( throw_str(d) )