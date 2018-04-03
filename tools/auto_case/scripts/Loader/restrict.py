import os
import io
import sys
import time
import traceback

from utility import flatitem
from config import option
from Session import make, name
from google.protobuf.descriptor import FieldDescriptor

def _get_field( node, *fields : tuple ):
	'''
	根据给出的根配置，取子配置，不查找模板\n
	'''
	if len(fields) == 0:
		return node
	
	field, *fields = fields
	if type(node) is dict and field in node.keys():
		return _get_field( node[field], *fields )

	return None 

def _get_field2( root, *fields : tuple ):
	'''
	根据给出的根配置，取子配置\n
	子配置不存在则查找根配置的模板\n
	'''
	rv = _get_field( root, *fields )

	if rv is None:
		# 获取继承关系
		inherit = root.get('inherit')
		rv = _get_field2( inherit, *fields ) if inherit else None
	
	return rv

# 异常的获取函数
if hasattr(sys, '_getframe'):
	currentframe = lambda: sys._getframe(2)
else: #pragma: no cover
	def currentframe():
		"""Return the frame object for the caller's stack frame."""
		try:
			raise Exception
		except Exception:
			return sys.exc_info()[1].tb_frame.f_back

# 调用栈的信息
def stackinfo(stack=False):
	f = currentframe()
	#On some versions of IronPython, currentframe() returns None if
	#IronPython isn't run with -X:Frames.
	if f is not None:
		f = f.f_back
	rv = "(unknown file)", 0, "(unknown function)", None
	if hasattr(f, "f_code"):
		co = f.f_code
		sinfo = None
		if stack:
			sio = io.StringIO()
			sio.write('\nStack (most recent call last):\n')
			traceback.print_stack(f, file=sio)
			sinfo = sio.getvalue()
			if sinfo[-1] == '\n':
				sinfo = sinfo[:-1]
			sio.close()
		rv = (os.path.basename(co.co_filename), f.f_lineno, co.co_name, sinfo)
	return rv


class exception(Exception):
	'''
	约束检查的异常类
	'''

	def __init__(self, value, msg, inf = None):
		self.val = value
		self.msg = msg
		self.inf = inf

	def __str__(self):
		return "%s" % (repr(self.msg))

	def value(self):
		return self.val

	def message(self):
		return self.msg

	def file(self):
		return self.inf[0] or 'unknowe file' if self.inf else 'unknowe file'

	def line(self):
		return self.inf[1] or 0 if self.inf else 0

	def func(self):
		return self.inf[2] or 'unknowe function' if self.inf else 'unknowe function'

	def trackback(self):
		return self.inf[3] or '' if self.inf else ''

	@staticmethod
	def throw( code, message, stack=False ):
		raise exception( code, message, stackinfo(stack) )


class leaf:

	def __init__(self):
		# 父节点
		self.parent = None

	def parent(self):
		return self.parent

	def check(self, msgid, msg, cli):
		'''
		检查节点是否命中
		'''
		return False

class message(leaf):
	'''
	约束检查
	'''
	def __init__(self, msgid, check=None, count=1, **kw):
		'''
		初始化约束
		cdata - 数据检查回调
		count - 所需命中次数
		'''
		super().__init__()
		# 消息ID
		self.msgid = eval(msgid) if type(msgid) is str else msgid
		# 消息命中次数
		self.count = count
		# 消息计数
		self.hit = 0
		# 数据检查回调
		self.cdata = check

	def reset(self):
		self.hit = 0

	def check(self, msgid, msg, cli):
		'''
		检查是否通过验证
		@return 是否处理，验证结果
		'''
		if self.msgid == msgid:
			# 没有数据检查则返回已处理，且验证通过
			if self.cdata: 
				for c in flatitem( self.cdata ):
					if False == c(cli, session = cli.session, msg = msg):
						fname, _ = c.__qualname__.split(".",1)
						if fname == "verify":
							exception.throw(-1, "message verify failed. [%s != %s]" % tuple(str(v.cell_contents) for v in c.__closure__))
				else:
					self.hit += 1
			else:
				self.hit += 1

			return True

		return False  # 未命中

	def is_finished(self):
		return self.hit == self.count

class node(leaf):
	'''
	约束节点的基类
	'''

	def __init__(self):
		super().__init__()
		self.children = []

	def append(self, *args):
		for arg in args:
			arg.parent = self
			self.children.append( arg )

	def insert(self, *args, idx=0):
		for i, arg in enumerate(args):
			arg.parent = self
			self.children.insert( idx + i, arg )

	def empty(self):
		self.children = []

	def front(self):
		return self.children[0]

	def back(self):
		return self.children[-1]

	def pop(self):
		v = self.children[0]
		del children[0]

		return v

	def push(self, v):
		v.parent = self
		self.children.insert(0, v)

	def push_back(self, *v):
		self.append(*v)

	def push_front(self, *v):
		self.insert(*v)

	def pop_front(self):
		del self.children[0]

	def pop_back(self):
		del self.children[-1]

	def is_empty(self):
		return len(self.children) == 0

	def size(self):
		return len(self.children)

	def reset(self):
		for child in self.children:
			child.reset()

		self.children = []

	def is_finished(self):
		return self.is_empty()

class sequence_node(node):
	'''
	序列节点
	'''

	def __init__(self, **kw):
		super().__init__()

	def check(self, msgid, msg, cli):
		'''
		@brief 只访问第一个元素，失败就跳出
		@param msgid 消息号
		@param msg 消息数据
		@param cli 客户端连接对象

		@return 该消息是否命中
		'''
		r = self.children[0]
		if r.check(msgid, msg, cli):
			if r.is_finished():
				del self.children[0]

			return True

		return False


class collect_node(node):
	'''
	收集节点
	'''

	def __init__(self, **kw):
		super().__init__()

	def check(self, msgid, msg, cli):
		'''
		不分顺序，全部收集后验证通过
		@param msgid 消息号
		@param msg 消息数据
		@param cli 客户端连接对象

		@return 该消息是否命中
		'''
		for i in range(len(self.children)):
			r = self.children[i]
			if r.check(msgid, msg, cli):
				if r.is_finished():
					del self.children[i]

				return True

		return False


class switch_node(node):
	'''
	选择节点
	'''
	def __init__(self, **kw):
		super().__init__()
		self.active = None

	def check(self, msgid, msg, cli):
		'''
		分支节点，任意节点命中后直到节点完成
		@param msgid : 消息号
		@param msg   : 消息数据
		@param cli   : 客户端连接对象
		@return      : 该消息是否命中
		'''
		if self.active is not None:
			# 已命中且未结束则继续该分支
			if self.active.check(msgid, msg, cli):
				if self.active.is_empty():
					self.children = []

				return True
		else:
			# 否则查找命中的分支
			for r in self.children:
				if r.check(msgid, msg, cli):
					if r.is_finished():
						self.children = []
					else:
						self.active = r

					return True

		return False

class restrict:
	def __init__(self, conf):
		# yapf: disable
		# 约束超时时间
		self.timeout = _get_field(conf, 'timeout') or 0
		# 约束期间内忽略的消息
		ignores = _get_field(conf, 'ignores')
		self.ignore_all = True if type(ignores) is str and ignores == "*" else False
		if not self.ignore_all:
			self.ignores = [eval(v) for v in flatitem(ignores) if type(v) in (int,str)]

		# 重新计算超时时间
		self.timeout = 0 if self.timeout == 0 else time.clock() + self.timeout

		# 定义递归节点生成函数
		def make_node( conf ):
			'''
			根据节点配置生成消息约束树
			'''
			if not conf:
				return None

			class_ = conf.get('class')
			if not class_:
				return None

			class_ = globals().get( class_ )
			if not class_:
				return None

			if class_ and issubclass(class_, leaf):
				node = class_( **conf )
				for c in conf.get('child') or ():
					child = make_node( c )
					if child:
						node.append( child )

				return node
			
			return None

		# 生成节点
		self.node = make_node(conf)

		# yapf: enable

	def check(self, cli, msgid, msg):
		'''
		#brief 检查约束
		#param msgid 消息索引
		
		#return 该约束是否已完成
		'''
		# 忽略部分消息
		if self.ignores and msgid in self.ignores:
			return False

		if self.node:
			if self.node.check(msgid, msg, cli):
				# 检查节点是否命中
				return self.node.is_finished()
			
			if self.ignore_all:
				# 全部忽略的情况，则认为未完成
				return False
			else:
				# 否则认为约束不成立，抛出异常
				exception.throw(-2, 'message id was not hit.')

		return self.node.is_finished()

	def check_timeout(self):
		'''
		检查是否超时
		'''
		if self.timeout > 0 and time.clock() > self.timeout:
			exception.throw( 1, "restrict timeout." )

	def finished(self):
		return self.test in ['success', 'failed']
		
def __set_repeated( msg, key, val, convert ):
	# 对 repeated 属性的字段赋值
	setattr(msg, key, [convert(v) for v in val])

def __set_optional( msg, key, val, convert ):
	# 对 optional 属性的字段赋值
	setattr(msg, key, val)

def __set_message( cli, msg, val ):
	'''
	msg - protobuf message instance
	val - node of message field value
	'''
	des = msg.DESCRIPTOR

	# 取出字段名
	fields = [f.name for f in des.fields]
	for k, v in val.items():
		# 遍历配置中的字段，并赋值
		v = v(cli, session = cli.session) if callable(v) else v # 可执行的函数，则执行后取返回值

		field_descriptor = des.fields_by_name.get(k)

		if field_descriptor:
			if field_descriptor.label == FieldDescriptor.LABEL_REPEATED:
				# repeated type
				set_value = __set_repeated
			else:
				# optional type
				set_value = __set_optional

			if field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_INT32:
				# int type
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_INT64:
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_UINT32:
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_UINT64:
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_DOUBLE:
				set_value( msg, k, v, float )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_FLOAT:
				set_value( msg, k, v, float )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_BOOL:
				set_value( msg, k, v, bool )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_ENUM:
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_STRING:
				set_value( msg, k, v, str )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_MESSAGE:
				# this is a message type
				sub = getattr( msg, k )
				__set_message( cli, sub, v )
		else:
			WRN( '%s is not messgae field.' % (k) )
			return False

	return True


def trigger( cli, name ):
	'''
	生成后续的消息约束
	name : 测试名称
	'''

	# 设置测试用例
	package = cli.package
	session = cli.session

	account.debug( "~ trigger %s" % (name) )

	if name is None: 
		account.debug( "~ trigger name is None" )
		return False

	if cli.package is None:
		account.debug( "~ trigger package is None" )
		return False

	cli.test = name
	cli.befor = None
	cli.after = None
	cli.throw = None
	cli.timeout = 0
	cli.ignores = ()
	cli.restrict = None

	# 设置当前测试配置
	test_conf = _get_field2(package, name)
	if test_conf is None:
		# 查一下该测试集中有没有该测试，查到就继续，否则换下一个测试集
		return False
	
	# 获取约束节点，遵循继承定义
	recv_conf = _get_field2(package, name, 'recv')

	# 生成消息约束
	cli.restrict = None if recv_conf == None else restrict( recv_conf )
	# 消息约束
	account.debug( "~ trigger %s create restrict node." % (name) )

	# 约束通过后的检查函数，该函数返回分支名
	cli.after = _get_field2(package, name, 'after')
	# 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
	cli.throw = _get_field2(package, name, 'throw') or 'failed'

	# 进行准备工作，准备工作会自动调用继承测试的准备工作集
	for func in flatitem(_get_field2(package, name, 'befor')):
		#account.debug('! execute befor : {%s}' % (func.code))
		func(cli, session = cli.session, option = option)

	# 发送字段可以发送消息
	send = _get_field2(package, name, 'send')
	if send:
		# send 字段不为空，则根据配置填充一个 message buffer 并发送
		msgid = _get_field2( package, name, 'send', 'msgid' )
		field = _get_field2( package, name, 'send', 'field' )
		# 转化 msgid
		msgid = eval(msgid) if type(msgid) is str else msgid
		msg = make( msgid )
		if msg and __set_message(cli, msg, field):
			cli.session.Send( msgid, msg )

	if cli.restrict is None:
		# 此处不可使用node来做判定，因为execute会执行prepare中的函数，有改变restrict.node的可能
		return trigger(cli, check_data(cli))

	return True

def check(cli, msgid, msg):
	'''
	检查消息约束
	'''
	# 判断约束是否完成
	if cli.restrict and cli.restrict.check(cli, msgid, msg):
		# 完成则执行后续动作
		trigger(cli, check_data(cli))

def check_data(cli):
	# 设置运行环境
	session = cli.session

	after = cli.after
	# 若没有则返回默认的throw
	throw = cli.throw
	# 字典类型则检查每一个给出的字段
	if after:
		for check in flatitem( after ):
			# 判定验证类型
			if not callable(check): 
				continue
			
			# lambda类型则直接调用，对应配置中的check_callback
			if not check(cli, session = cli.session):
				exception.throw(-3, '%s after check failed.' % (cli.test))
		else:
			# 验证通过了
			account.info("~ %s after check success!" % (cli.test))
	
	return throw if type(throw) is str else throw()

def var(v):
	def f(cli, **kwargs):
		_loc = locals()
		_loc.update(kwargs)
		_obj, name = v.split('.', 1)
		_obj = _loc.get(_obj)

		return getattr(_obj, name)

	return f

def assign(var, val):
	def f(cli, **kwargs):
		_loc = locals()
		_loc.update(kwargs)
		_obj, name = var.split('.', 1)
		_obj = _loc.get(_obj)

		setattr(_obj, name, val(cli, **kwargs) if callable(val) else val )
		return True

	return f
	
def invoke(code):
	def f(cli, **kwargs):
		locals().update(kwargs)
		return eval(code)

	return f
