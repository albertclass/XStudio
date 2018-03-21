import sys
import inspect
import restrict
import traceback
import importlib

from itertools import zip_longest
from Widgets import *
from Session import name, make

def get_field( node, *fields : tuple ):
	if len(fields) == 0:
		return node
	
	field, *fields = fields
	if type(node) is dict and field in node.keys():
		return get_field( node[field], *fields )

	return None

def get_field2( node, *fields : tuple ):
	rv = get_field( node, *fields )

	if rv is None:
		# 获取继承关系
		inherit = None if 'inherit' not in node else node['inherit']

		if inherit is not None:
			return get_field2( inherit, *fields )
	
	return rv


def show_node(value):
	if 'instance' in value:
		instance = value['instance']
		if isinstance(instance, restrict.node):
			return '%s.%s()' % (instance.__module__, type(instance).__name__)

	return ''

def show_text(value):
	return str(value)

def show_inherit(value):
	return value['__name__']

def show_prepare(value):
	if inspect.isfunction(value):
		return show_func(value)
	
	return ''

def show_func(value):
	if value is None:
		return 'None'
	
	if not callable(value):
		return 'Error'

	if hasattr(value, '__objclass__'):
		return '.'.join( (value.__objclass__.__name__, value.__name__) )
	elif value.__name__ == '<lambda>':
		lines, lnum = inspect.getsourcelines(value)
		lines = lines[0]
		start = lines.find('lambda')
		colon = lines.find(':', start)
		close = colon + 1

		match = 0
		for c in lines[colon+1:]:
			if match == 0 and c == ',':
				break

			if c in '[(':
				match += 1
			elif c in ')]':
				match -= 1
			
			if match < 0:
				break

			close += 1

		return lines[start:close].strip()
	else:
		sig = inspect.signature(value)
		args = [(p.name, p.default) for p in sig.parameters.values()]
		param = ''
		params = []
		for arg in args:
			param += arg[0]
			if arg[1] != inspect.Parameter.empty:
				default = getattr(arg[1], '__name__', None)
				if default:
					param += str(default)

			params.append(param)

		return '%s(%s)' % ( value.__name__, ', '.join(params) )

def show_inspect(value):
	if type(value) is not restrict.inspect:
		return ''
	
	param = show_flat(value.msgid, value.cdata, value.logic)
	return 'restrict.inspect(%s)' % (param)

def show_msgid(value):
	msgid = int(value)
	return 'ids.' + ids.names[msgid]

def show_list(value):
	return show_some(value)

def show_tuple(value):
	return show_some(value)

def show_flat(*args, joinwith=', '):
	some = []
	for arg in args:
		if arg is None:
			continue

		if inspect.isroutine(arg):
			some.append(show_func(arg))
		elif type(arg) is tuple:
			some.append(joinwith.join([show_some(x) for x in arg]))
		elif type(arg) is list:
			some.append(joinwith.join([show_some(x) for x in arg]))
		elif type(arg) is dict:
			some.append(joinwith.join([show_some(x) for x in arg.items()]))
		elif type(arg) is str:
			some.append('\'' + arg + '\'')
		else:
			some.append(str(arg))
	
	return joinwith.join(some)

def show_some(*args, joinwith=', '):
	some = []
	for arg in args:
		if inspect.isroutine(arg):
			some.append(show_func(arg))
		elif type(arg) is tuple:
			some.append('('+ joinwith.join([show_some(x) for x in arg]) +')')
		elif type(arg) is list:
			some.append('['+ joinwith.join([show_some(x) for x in arg]) +']')
		elif type(arg) is dict:
			some.append('{'+ joinwith.join([show_some(x) for x in arg.items()]) + '}')
		elif type(arg) is str:
			some.append('\'' + arg + '\'')
		else:
			some.append(str(arg))
	
	return joinwith.join(some)

def build_by_node(tree, conf, node, vars, name):
	if 'instance' not in vars:
		return

	vtype = type(vars['instance'])
	for c in [_ for _ in conf['children'] if _['type'] is vtype]:
		if 'children' in vars:
			show = '' if 'show' not in conf else conf['show'](vars)
			sort = 'end' if 'sort' not in conf else conf['sort']

			item = tree.insert(node, sort, text=c['text'], tags=c['tags'], values=[show, name, vtype]) # 添加节点

			for child in vars['children']:
				build_by_node(tree, conf, item, child, None)
		
		if 'children' in c:
			build_conf(tree, c, node, vars['instance'])


def build_by_dict(tree, conf, node, vars, name):
	# 字典类型的子项
	conf_d = { c['name'] : c for c in conf['children'] } # trans to dict

	for k, v in vars.items():
		if k in conf_d:
			build_conf(tree, conf_d[ k ], node, v, k)
		elif '*' in conf_d:
			build_conf(tree, conf_d['*'], node, v, k)
		else:
			pass

def build_by_type(tree, conf, node, vars, name):
	conf_d = {}
	for c in conf['children']:
		# 遍历所有子项配置
		ctype = c['type']
		if type(ctype) in (tuple, list):
			# 多种类型
			conf_d.update( { t : c for t in ctype } )
		else:
			# 单一类型
			conf_d[c['type']] = c

	for v in vars:
		# 根据值类型匹配
		if type(v) not in conf_d:
			continue
		
		build_conf(tree, conf_d[type(v)], node, v )

def build_by_list(tree, conf, node, vars, name):
	if 'children' not in conf:
		return
	
	conf_d = conf['children']

	for i in range(min(len(conf_d), len(vars))):
		build_conf(tree, conf_d[i], node, vars[i])

def build_by_bind(tree, conf, node, vars, name):
	if 'children' not in conf:
		return

	for c in conf['children']:
		build_conf(tree, c, node, getattr(vars, c['name'], None), c['name']) 


def build_by_value(tree, conf, node, vars, name):
	pass


def build_conf(tree, conf, node, vars, name=''):
	if 'text' not in conf:
		return

	if 'tags' not in conf:
		return

	if 'mode' not in conf:
		return
	
	ctype = conf['type']
	vtype = type(vars)

	if vtype is ctype if inspect.isclass(ctype) else vtype in ctype:
		show = '' if 'show' not in conf else conf['show'](vars)
		sort = 'end' if 'sort' not in conf else conf['sort']
		item = tree.insert(node, sort, text=conf['text'] or name, tags=conf['tags'], values=[show, name, vtype]) # 添加节点
		while 'edit' in conf:
			col, flag, options = [x for x, _ in zip_longest(conf['edit'], range(3))]
		
			if flag == 'text':
				tree.inplace_entry(col, item)
				break
			
			if flag == 'spin':
				if options and 'args' in options:
					tree.inplace_spinbox(col, item, *options['args'])
				else:
					tree.inplace_spinbox(col, item, from_=0, to=100, increment=1)

				break

			if flag == 'message':
				widget = SuggestEntry(tree, values=('1','2'), textvariable=tk.StringVar())
				tree.inplace_custom(col, item, widget)
				break

			if flag == 'function':
				tree.inplace_entry(col, item)
				break
			
			break

		build_by_user = None if 'build' not in conf else conf['build']
	
		if callable(build_by_user):
			build_by_user(tree, conf, item, vars, name)
		elif vtype is dict:
			build_by_dict(tree, conf, item, vars, name)
		elif vtype is list:
			build_by_type(tree, conf, item, vars, name)
		elif vtype is tuple:
			build_by_type(tree, conf, item, vars, name)
		else:
			build_by_value(tree, conf, item, vars, name)

class CaseBuilderException(Exception):
	def __init__(self, code, message):
		self.__code = code
		self.__message = message

	def code(self):
		return self.__code

	def message(self):
		return self.__message

	def __str__(self):
		return (repr(self.__message))

	@staticmethod
	def throw(code, message):
		raise CaseBuilderException(code, message)

class CaseBuilder:
	def __init__(self, tree, name, template):
		self.tree = tree
		self.case = None

		# 测试模块
		self.module = self.load(name + '.case', True)
		# 配置描述
		self.template = template

		# 测试包
		self.case_package = self.module.case_package if self.module else None
		# 已导入的case列表
		self.case_import = []

	def load(self, name, reload=False):
		module = None
		try:
			if reload and name in sys.modules:
				module = importlib.reload(sys.modules[name])
			else:
				module = importlib.import_module(name)
		except SyntaxError as error:
			traceback.print_exc()

		return module

	def build(self, rebuild=False):
		'''
		根据配置创建树界面
		'''
		if not self.case_package:
			return False

		if rebuild:
			# 重建，清理所有根节点
			items = self.tree.get_children()
			self.tree.delete( *items )

		for case in self.case_package:
			self.build_case(case)

		return True

	def build_case(self, case):
		'''
		根据配置创建测试节点
		'''
		if case in self.case_import:
			return

		if case is common.basic_case:
			case['__name__'] = 'common.basic_case'
			return

		if 'inherit' in case:
			self.build_case(case['inherit'])

		for k, v in self.module.__dict__.items():
			if v is case:
				v['__name__'] = k
				break
		else:
			return

		print(case['__name__'])
		self.case = case
		self.case_import.append(case)
		
		# 根据配置项创建
		build_conf(self.tree, self.template, '', case, name=case['__name__'])

#################################################################
class CaseParser:
	def __init__(self, indent='\t'):
		self.indent_deep = 0
		self.indent_char = indent
		self.lines = [
			'import restrict',
			'import message.ids as ids',
			'import message.client_msg as client_msg',
			'import message.common as common',
			'from .func import *',
			''
		]

	def indent(self, n):
		self.indent_deep += n
	
	def write(self, *args, indent=True):
		something = ''.join( args )
		if something:
			lines = something.split('\n')
			self.lines[-1] += lines[0]

			for line in lines[1:]:
				if indent:
					self.lines.append(self.indent_char*self.indent_deep + line)
				else:
					self.lines.append(line)


	def dump(self, ends='\n'):
		return ends.join(self.lines)

	def prase(self, tree, node, conf):
		children = tree.get_children(node)
		for child in children:
			self.write('\n%s = ' % tree.item(child, option='text'))
			prase_conf( tree, child, conf, self)
		
def prase_by_empty(tree, node, conf, writer):
	pass

def prase_by_dict(tree, node, conf, writer):
	writer.write('{')
	writer.indent(+1)
	children = tree.get_children(node)
	for child in children:
		writer.write("\n'%s' : " % (tree.set(child, '2')))
		for c in conf['children']:
			if tree.tag_has(c['tags'], child):
				prase_conf(tree, child, c, writer)
				break

		writer.write(',')
	
	writer.indent(-1)
	writer.write('\n}')

def prase_by_list(tree, node, conf, writer):
	writer.write('[')
	writer.indent(+1)
	children = tree.get_children(node)
	for child in children:
		for c in conf['children']:
			if tree.tag_has(c['tags'], child):
				writer.write('\n')
				prase_conf(tree, child, c, writer)
				writer.write(',')
				break

	writer.indent(-1)
	writer.write('\n]')

def prase_by_list_singleline(tree, node, conf, writer):
	writer.write('[')
	children = tree.get_children(node)
	for child in children:
		for c in conf['children']:
			if tree.tag_has(c['tags'], child):
				prase_conf(tree, child, c, writer)
				writer.write(',')
				break

	writer.write(']')

def prase_by_tuple(tree, node, conf, writer):
	writer.write('(')
	writer.indent(+1)

	children = tree.get_children(node)
	for child in children:
		for c in conf['children']:
			if tree.tag_has(c['tags'], child):
				writer.write('\n')
				prase_conf(tree, child, c, writer)
				writer.write(',')
				break

	writer.indent(-1)
	writer.write('\n)')

def prase_by_tuple_singleline(tree, node, conf, writer):
	writer.write('(')
	children = tree.get_children(node)
	for child in children:
		for c in conf['children']:
			if tree.tag_has(c['tags'], child):
				prase_conf(tree, child, c, writer)
				writer.write(',')
				break

	writer.write(')')

def prase_by_text(tree, node, conf, writer):
	text = tree.set(node, '1')
	writer.write(text)

def prase_by_string(tree, node, conf, writer):
	text = tree.set(node, '1')
	writer.write("'", text, "'")

def prase_by_node(tree, node, conf, writer):
	children = tree.get_children(node)
	for i, child in enumerate(children):
		text = tree.set(child, '3')
		writer.write(',\n\n{' if i else '{')
		writer.indent(+1)
		for c in filter( lambda c : str(c['type']) == text, conf['children'] ):
			writer.write("\n'instance' : ")
			prase_conf(tree, child, c, writer)
			writer.write(',')
			if c['tags'] == 'node':
				writer.write("\n'children' : (")
				writer.indent(+1)
				writer.write('\n')
				prase_conf(tree, child, conf, writer)
				writer.indent(-1)
				writer.write("\n)")
			break;
		
		writer.indent(-1)
		writer.write("\n}")


def prase_by_bind(tree, node, conf, writer):
	writer.write('%s.%s(' % (conf['type'].__module__, conf['type'].__name__))
	children = tree.get_children(node)
	params = []
	for child in children:
		params.append(tree.item(child, option='values'))

	params_str = ', '.join('%s = %s' % (name, text) for text, name, stype in params)
	writer.write(params_str, ')') 

def prase_conf(tree, node, conf, writer):
	if 'tags' not in conf:
		return

	ctype = conf['type']
	vtype = tree.set(node, '3')

	prase_by_user = None if 'prase' not in conf else conf['prase']
	if callable(prase_by_user):
		prase_by_user(tree, node, conf, writer)
	elif vtype == str(dict):
		prase_by_dict(tree, node, conf, writer)
	elif vtype == str(list):
		prase_by_list(tree, node, conf, writer)
	elif vtype == str(tuple):
		prase_by_tuple(tree, node, conf, writer)
	elif vtype == str(str):
		prase_by_string(tree, node, conf, writer)
	elif vtype == str(type(lambda x: x)):
		prase_by_text(tree, node, conf, writer)
	else:
		prase_by_text(tree, node, conf, writer)
