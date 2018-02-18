import os
import os.path
import time

from collections import Iterable

def make_path( path, root ='.', slash='/' ):
	dir_list = path.split('/')
	path = root
	for dir in dir_list:
		path += '/' + dir  
		if not os.path.exists(path):
			os.mkdir(path)

def flatiter(seq, ignore_types=(str, bytes), deep = -1):
	if deep == 0:
		yield seq
		return
	
	for x in seq:
		if isinstance(x, Iterable) and not isinstance(x, ignore_types):
			yield from flatiter(x, ignore_types, deep - 1)
		else:
			yield x

def flatitem(obj, ignore_types=(str, bytes), deep = -2):
	return flatiter((obj,) if obj else (), deep = deep + 1 )

__indent = '    '
__loging = ''

def format_output( fmt, *args : tuple ):
	global __loging
	__loging += fmt % args

def format_tuple( arg : tuple, deep, output ):
	output('(')
	indent = __indent * deep
	for i in arg:
		output( '\n' + indent + __indent )
		format_value(i, deep+1, output)
		output( ',' )

	output( indent + ')' )

def format_list( arg : list, deep, output ):
	indent = __indent * deep
	output('[')
	for i in arg:
		output( '\n' + indent + __indent )
		format_value(i, deep+1, output)
		output( ',' )

	output( '\n' + indent + ']')

def format_dict( arg : dict, deep, output ):
	indent = __indent * deep
	output('{')
	for k, v in arg.items():
		output( '\n' + indent + __indent )
		format_key(k, output)
		output( ' : ')
		format_value(v, deep+1, output )
		output( ',' )
	output( '\n' + indent + '}')

def format_key( arg, output ):
	t = type(arg)
	if t in (tuple, list, dict):
		output( str(t) + ' : %08x'%(hex(id(arg))))
	elif t in ( bool, int, float, complex, str ):
		output( str(arg) )
	elif inspect.isfunction(t):
		output( str(arg) )
	elif inspect.isclass(t):
		output( str(arg) )
	else:
		output( str(arg) )

def format_value(arg, deep, output):
	t = type(arg)
	if t is tuple:
		format_tuple( arg, deep, output )
	elif t is list:
		format_list( arg, deep, output )
	elif t is dict:
		format_dict( arg, deep, output )
	elif t in ( bool, int, float, complex, str ):
		output( str(arg) )
	elif inspect.isfunction(t):
		output( str(arg) )
	elif inspect.isclass(t):
		format_dict( arg.__dict__, deep, output )
	else:
		return arg

def output_value(arg, output = format_output):
	global __loging
	__loging = ''
	format_value(arg, 0, output)
	account.info( __loging + '\n' )
