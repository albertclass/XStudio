import network
import report
import restrict

from Session import hook, make, name

class Client:
	def __init__(self, package, session, hooktbl):
		# 网络连接
		self._net = {}
		self._new = {}
		self._net_for = False

		# 消息挂钩
		self._hook_tbl = hooktbl
		# 报表
		self._report = report.Report()

		# 约束节点
		self.restrict = None
		# 会话
		self.session = session(self)
		# 测试集
		self.package = package

	def __del__(self):
		for net in self._net.values():
			net.close()

		self._net = {}

	def setup_report(self, file, level=0, header=report.report_header):
		self._report.setup(file, level, header)

	def close_report(self):
		self._report.close()
		
	def report(self, **kw):
		self._report.record_some( **kw )

	def setup_logger(self):
		pass

	def close_logger(self):
		pass
	
	def connect(self, idx, host, port):
		net = self._net.get(idx)
		if net:
			net.close()
			del net

		net = network.client(self.session)
		if net and net.connect((host,port)):
			if self._net_for:
				self._new[idx] = net
			else:
				self._net[idx] = net

	def disconnect(self, idx):
		net = self._net.get(idx) or self._new.get(idx)
		if net:
			net.close()
			del net

	def connected(self, idx):
		net = self._net.get(idx) or self._new.get(idx)
		return net and net.connected()

	def network(self, idx):
		return self._net.get(idx) or self._new.get(idx)

	def session(self):
		return self._session

	def loop(self):
		idel = False
		# 超时检查
		self.restrict.check_timeout()

		self._net_for = True
		for net in self._net.values():
			ret = net.recv()
			if ret == -1:
				break
			elif ret == -2:
				self.report(Log='reconnect')
				self.error("recv error reconnect now.")
				net.reconnect()
				continue
			else:
				idel = True

		self._net_for = False
		if len(self._new) > 0:
			self._net.update(self._new)
			self._new.clear()

		return idel

	def dispatch(self, msgid, msg):
		# 根据消息表处理消息，优先处理自定义消息
		# INF('# custom hook table handled, msgid = %d' % msgid)
		handler = self._hook_tbl.get(msgid) or hook.hook_tbl.get(msgid)

		# 继续处理逻辑
		if callable(handler):
			# 打印消息索引和对应的处理句柄
			account.info("# %s.%s hook message : %s - %d" % (handler.__module__, handler.__name__, name(msgid), msgid))
			handler(self, msg)
		else:
			WRN('# dispatch message failed, msgid = %d' % msgid)

		restrict.check( self, msgid, msg )


