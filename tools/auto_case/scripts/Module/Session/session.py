import report

from . import hook, make, name

class session:
	'''
	session object for network
	'''
	def __init__(self, net, hook_tbl):
		self._hook_tbl = hook_tbl
		self._net = net
		self._report = report.Report()

	def setup_report(self, file, level=0, header=report.report_header):
		self._report.setup(file, level, header)

	def close_report(self):
		self._report.close()
		
	def report(self, **kw):
		self._report.record_some( **kw )
	
	def connect(self, host, port ):
		self._net.connect( (host, int(port) ) )

	def close(self):
		self._net.close()

	def is_connected(self):
		return self._net.connected()

	def onParse(self, buffer):
		'''
		parse buffer and get message package length.
		'''
		return struct.unpack("!H", buffer[:2])

	def setup_logger(self):
		pass

	def close_logger(self):
		pass

	def onRecv(self, net, data):
		'''
		call with whole package received.
		'''
		hsize = struct.calcsize("!2H")
		psize, msgid = struct.unpack("!2H", data[:hsize])

		msg = make(msgid)
		if msg:
			msg.ParseFromString(data[hsize:])
		else:
			WRN("# message parser does't found. %s" % name(msgid))
			return

		# 再根据消息表处理消息，优先处理自定义消息
		# INF('# custom hook table handled, msgid = %d' % msgid)
		handler = self._hook_tbl.get(msgid) or hook.hook_tbl.get(msgid)
		if not handler:
			WRN('# dispatch message failed, msgid = %d' % msgid)

		# 继续处理逻辑
		if callable(handler):
			# 打印消息索引和对应的处理句柄
			account.info("# %s.%s hook message : %d" % (handler.__module__, handler.__name__, msgid))
			handler(self, msg)

		restrict.check( msgid, msg, net )


	def Send(self, msgid, msg):
		data = struct.pack(
			"!2H%ds" % len(buf), len(buf), msgid, msg.SerializeToString() if msg else bytes())
			
		net.send(data)

class package:

	def __init__(self, user):
		self.user = user
		self.slots = []

	def append(self, slot):
		self.slots.append(slot)

	def delete(self, slot):
		self.slots.remove(slot)

	def find(self, comp):
		for slot in self.slots:
			if comp(slot):
				return slot

		return None

	def cleanslots(self):
		self.slots = []


class role(session):

	def __init__(self, net, hook_tbl):
		super().__init__(net, hook_tbl)
		self.name = ''
		self.gid = 0
		self.uid = 0

		# 生成管理对象
		'''
		self.cards = package(self)
		self.items = package(self)
		self.mails = package(self)
		'''

	def set_name(self, name):
		self.name = name

	def get_name(self):
		return self.name
