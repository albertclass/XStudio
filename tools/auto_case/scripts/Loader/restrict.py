import os
import sys
import time
from utility import flatitem
from config import option
from Session import make, name
from google.protobuf.descriptor import FieldDescriptor

def _get_field( node, *fields : tuple ):
	if len(fields) == 0:
		return node
	
	field, *fields = fields
	if type(node) is dict and field in node.keys():
		return _get_field( node[field], *fields )

	return None 

def _get_field2( root, *fields : tuple ):
	rv = _get_field( root, *fields )

	if rv is None:
		# 获取继承关系
		inherit = None if 'inherit' not in root else root['inherit']

		if inherit is not None:
			rv = _get_field2( inherit, *fields )
	
	return rv

class exception(Exception):
	'''
	约束检查的异常类
	'''

	def __init__(self, value, msg):
		self.val = value
		self.msg = msg

	def __str__(self):
		return "%s" % (repr(self.msg))

	def value(self):
		return self.val

	def message(self):
		return self.msg

	@staticmethod
	def throw( code, message ):
		raise exception( code, message )


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
	def __init__(self, msgid, check=None, count=1):
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
				if callable(self.cdata) and self.cdata():
					self.hit += 1
				else:
					exception.throw( -1, '%s check data failed' % (self.cdata.__name__))
			else:
				self.hit += 1

			return True

		return False  # 未命中

	def is_finished(self):
		return True

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

	def __init__(self):
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

	def __init__(self):
		super().__init__()

	def check(self, msgid, msg, cli):
		'''
		@brief 不分顺序，全部收集后验证通过
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

	def __init__(self):
		super().__init__()
		self.active = None

	def check(self, msgid, msg, cli):
		'''
		分支节点，任意节点命中后直到节点完成
		msgid : 消息号
		msg   : 消息数据
		cli   : 客户端连接对象
		return: 该消息是否命中
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
		self.ignores = _get_field(conf, 'ignores')

		# 重新计算超时时间
		self.timeout = 0 if self.timeout == 0 else time.time() + self.timeout

		# 消息约束
		account.debug( "~ trigger %s build restrict node." % (name) )

		# 定义递归节点生成函数
		def make_node( conf ):
			'''
			根据节点配置生成消息约束树
			'''
			if not conf:
				return None

			if 'instance' not in conf.keys():
				return None

			inst = [v for v in flatitem(conf['instance'], deep=1)]
			node = globals().get( inst[0] )
			if node and issubclass(node, leaf):
				node = node( **inst[1] if type(inst[1]) is dict else inst[1:] ) if len(inst) > 1 else node()
				for child_conf in conf.get('children') or ():
					child = make_node( child_conf )
					if child:
						node.append( child )

				return node
			
			return None

		# 生成节点
		self.node = make_node(conf)

		# yapf: enable

	def check_message(self, msgid, msg, cli):
		'''
		#brief 检查约束
		#param msgid 消息索引
		
		#return 该约束是否已完成
		'''
		# 忽略部分消息
		if self.ignores and msgid in self.ignores:
			return False

		if self.node:
			if self.node.check( msgid, msg, cli ):
				# 删除节点
				if isinstance( self.node, inspect):
					return True
				elif self.node.is_empty():
					return True
				else:
					return False
			elif '*' not in self.ignores:
				trigger(cli, 'failed' )
				return False
			else:
				return False

		return True

	def check_timeout(self):
		'''
		检查是否超时
		'''
		if self.timeout > 0 and time.time() > self.timeout:
			exception.throw( 1, "restrict timeout." )

	def finished(self):
		return self.test in ['success', 'failed']
		
def __set_repeated( msg, key, val, convert ):
	# 对 repeated 属性的字段赋值
	setattr(msg, key, [convert(v) for v in val])

def __set_optional( msg, key, val, convert ):
	# 对 optional 属性的字段赋值
	setattr(msg, key, val)

def __set_message( msg, val ):
	'''
	msg - protobuf message instance
	val - node of message field value
	'''
	des = msg.DESCRIPTOR

	# 取出字段名
	fields = [f.name for f in des.fields]
	for k, v in val.items():
		# 遍历配置中的字段，并赋值
		if callable(v): v = v(cli = cli, session=cli.session) # 可执行的函数，则执行后取返回值

		if k in fields:
			field_descriptor = des.field_by_name[k]

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
				set_value( msg, k, v, long )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_UINT32:
				set_value( msg, k, v, int )
			elif field_descriptor.cpp_type == FieldDescriptor.CPPTYPE_UINT64:
				set_value( msg, k, v, long )
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
				__set_message( sub, v )
		else:
			WRN( '%s is not messgae field.' % (k) )
			return False

	return True


def trigger( cli, name ):
	'''
	生成后续的消息约束
	case : 测试用例
	name : 测试名称
	'''

	# 设置测试用例
	case = cli.case
	session = cli.session

	account.debug( "~ trigger %s" % (name) )

	if name is None: 
		account.debug( "~ trigger name is None" )
		return False

	if cli.case is None:
		account.debug( "~ trigger case is None" )
		return False

	cli.test = name

	account.debug( "~ trigger %s build restrict." % (name) )
	# 设置当前测试配置
	test_conf = _get_field2(case, name)
	if test_conf is None:
		# 查一下该测试集中有没有该测试，查到就继续，否则换下一个测试集
		return False
	
	# 获取约束节点，遵循继承定义
	recv_conf = _get_field2(case, name, 'recv')

	# 生成消息约束
	cli.restrict = None if recv_conf == None else restrict( recv_conf )

	# 约束通过后的检查函数，该函数返回分支名
	cli.after = _get_field2(case, name, 'after')
	# 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
	cli.throw = _get_field2(case, name, 'throw') or 'failed'

	# 进行准备工作，准备工作会自动调用继承测试的准备工作集
	for code in flatitem(_get_field2(case, name, 'befor')):
		account.debug('execute code :"%s"' % (code))
		exec(code)

	# 发送字段可以发送消息
	send = _get_field2( case, name, 'send' )
	if send :
		# send 字段不为空，则根据配置填充一个 message buffer 并发送
		msgid = _get_field2( case, name, 'send', 'msgid' )
		field = _get_field2( case, name, 'send', 'field' )

		msg = make( msgid )
		if msg and __set_message(msg, field):
			cli.session.Send( msgid, msg )

	if cli.restrict is None:
		# 此处不可使用node来做判定，因为execute会执行prepare中的函数，有改变restrict.node的可能
		return trigger(cli, check_data(cli))

	return True

def check( msgid, msg, cli ):
	# 检查消息约束
	try:
		# 判断约束是否完成
		if cli.restrict and cli.restrict.check_message(msgid, msg, cli):
			# 完成则执行后续动作
			trigger(cli, check_data(cli))
	except exception as e:
		# 捕获到异常则认为失败，直接退出
		account.warning("restrict check raise a exception value = %d, message = %s" % ( e.value(), e.message()))
		trigger(cli, 'failed')

def check_data(cli):
	# 设置运行环境
	session = cli.session

	after = cli.after
	# 若没有则返回默认的throw
	throw = cli.throw
	# 字典类型则检查每一个给出的字段
	if 'check' in after.keys():
		for verify in flatitem( after['check'] ):
			# 判定验证类型
			# lambda类型则直接调用，对应配置中的check_callback
			if eval(verify):
				account.info("~ %s check success!" % (verify) )
				continue
			else:
				account.info("~ %s check failed!" % (verify) )
				break
		else:
			# 验证通过了，则使用验证通过的返回
			try:
				# 默认返回check配置中的throw
				return after['throw']
			except KeyError:
				return throw
	
	# 使用默认值
	return throw

# # Purely functional, no descriptor behaviour
# def partial(func, *args, **keywords):
# 	if hasattr(func, 'func'):
# 		args = func.args + args
# 		tmpkw = func.keywords.copy()
# 		tmpkw.update(keywords)
# 		keywords = tmpkw
# 		del tmpkw
# 		func = func.func

# 	@wraps(func)
# 	def newfunc(*fargs, **fkeywords):
# 		newkeywords = keywords.copy()
# 		newkeywords.update(fkeywords)
# 		return func(*(args + fargs), **newkeywords)
# 	newfunc.func = func
# 	newfunc.args = args
# 	newfunc.keywords = keywords
# 	return newfunc

def call(code):
	def invoke( **kwargs ):
		_locals = locals()
		for key, value in kw.items():
			_locals[key] = value

		return eval(code)

	return invoke
