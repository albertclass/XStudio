import struct

from . import name, make
class session:
	'''
	session object for network
	'''
	def __init__(self, cli):
		self._cli = cli

	def setNetwork(self, net):
		self._net = net
		return self

	def onParse(self, buffer):
		'''
		parse buffer and get message package length.
		'''
		return struct.unpack("!H", buffer[:2])

	def onRecv(self, data):
		'''
		call with whole package received.
		'''
		hsize = struct.calcsize("!2H")
		psize, msgid = struct.unpack("!2H", data[:hsize])

		msg = make(msgid)
		if not msg:
			WRN("# message parser does't found. %s" % name(msgid))
			return
		
		msg.ParseFromString(data[hsize:])
		
		self._cli.dispatch(msgid, msg)

	def Send(self, msgid, msg):
		'''
		send message to client\n
		@param msgid : message id\n
		@param msg : message data\n
		'''
		buf = msg.SerializeToString() if msg else bytes()
		dat = struct.pack("!2H%ds" % len(buf), len(buf) + 4, msgid, buf )
		
		self._net.send(dat)

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

	def __init__(self, cli):
		super().__init__(cli)
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
