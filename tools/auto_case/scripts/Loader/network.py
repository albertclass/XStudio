import sys
import socket
import select
import struct

class crc16:
	def __init__(self):
		self.crcvalue = 0
		self.upperTable = [ 0x0000,0x1231,0x2462,0x3653,0x48c4,0x5af5,0x6ca6,0x7e97,0x9188,0x83b9,0xb5ea,0xa7db,0xd94c,0xcb7d,0xfd2e,0xef1f ]
		self.lowerTable =[ 0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef ]

	def __del__(self):
		self.crc = 0

	def crc(self,value):
		high = self.crcvalue >> 12 & 0x0f
		low = self.crcvalue >> 8 & 0x0f
		tmp = self.crcvalue
		tmp = ((tmp & 0x00ff) << 8 ) | value
		tmp = (tmp ^ (self.upperTable[(high-1) +1] ^ self.lowerTable[(low-1) + 1 ] ))
		self.crcvalue = tmp

	def crc16(self,data):
		for j in data:
			self.crc(j)
		return self.crcvalue

class client:
	def __init__(self, case, session, hook_tbl ):
		self.recv_buf = bytes()
		self.connect_flag = False
		self.case = case
		self.restrict = None

		self.session = session(self, hook_tbl)

	def __del__(self):
		if self.s:
			del self.s

		del self.recv_buf

	def accept(self, server):
		self.s, self.address = server.s.accept()
		if not self.s:
			return False

		self.connect_flag = True
		return self.connected()

	def connect(self, address):
		self.address = address
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		return self.reconnect()

	def reconnect(self):
		try:
			# print( self.address )
			self.s.connect(self.address)
		except socket.error as e:
			WRN("connect to server failed err = %s" % e)
			return False

		self.s.setblocking(0)
		self.connect_flag = True
		self.recv_buf = bytes()
		return True

	def connected(self):
		return self.connect_flag

	def recv(self, ids = None):
		if False == self.connect_flag:
			account.error("------------------------------断开连接，客户端退出")
			return -1

		buffer_size = len(self.recv_buf)
		if buffer_size >= 2:
			# packet_size, = struct.unpack("!H", self.recv_buf[:2])
			packet_size, = self.session.onParse(self.recv_buf)
			# print( 'recv buffer len = %d, packet size = %d' % ( buffer_size, packet_size ) )
			if buffer_size >= packet_size:
				# print( 'data 1 %s' % repr( self.recv_buf ) )
				data = self.recv_buf[:packet_size]
				self.recv_buf = self.recv_buf[packet_size:]
				# print( 'data:%s' % repr( self.recv_buf ) )

				self.session.onRecv(self, data)
				return 1
		
		try:
			ifd, ofd, efd = select.select([self.s, ], [], [], 0)
			if len(ifd) != 0:
				recv_buf = self.s.recv(1024 * 16)
				if recv_buf:
					self.recv_buf += recv_buf

		except socket.error as e:
			print("recv data failed. err = %s" % e)
			self.close()
			self.connect_flag = False
			return -2, e.errno
		except:
			self.close()
			self.connect_flag = False
			return -2, e.errno

		return 0

	def send(self, data):
		try:
			# print( 'send data len = %d, data = %s' % ( len(data), repr(data) ) )
			self.s.sendall(data)
			
		except socket.error as e:
			self.session.error("send data failed. err = %s" % e)
			self.s.close()
			self.connect_flag = False
			return e.errno

		return 0

	def close(self):
		if self.s:
			if self.connected():
				self.s.shutdown(socket.SHUT_RDWR)
			self.s.close()

class server:
	def __init__(self, address):
		self.address = address
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	def __del__(self):
		if self.s:
			self.s.close()
			self.s = None

	def start(self):
		if not self.s:
			return False

		for res in socket.getaddrinfo(self.address[0], self.address[1], socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
			print("found interface :", res)
			af, socktype, proto, canonname, sa = res

			try:
				print("bind socket")
				self.s.bind(sa)
				print("listen ... ")
				self.s.listen(10)
			except socket.error as e:
				print("bind address failed! err = %s" % e)
				continue

		return True

	def stop(self):
		self.s.close()

# network test code
if __name__ == '__main__':
	if sys.argv[1] == 'client':
		cli = client()
		cli.connect(('127.0.0.1', 50007))

		while cli.connected():
			data = input("send data : ")
			print("send buffer : %s" % data)
			cli.send(0, data.encode())
			err, data = cli.recv()
			if err != 0:
				break

			while data:
				print("recv buffer : %s" % repr(data))
				err, data = cli.recv()

	elif sys.argv[1] == 'server':
		svr = server(('127.0.0.1', 50007))

		print("start server ...")
		if not svr.start():
			print('could not open socket')
			sys.exit(1)

		print("start server successful.")
		while True:
			print('ready a new connection.')
			cli = client()
			cli.accept(svr)
			if not cli.connected():
				print("server do not accept any client.")
				break

			print('connected by', cli.address)
			while True:
				err, data = cli.recv()
				if err != 0:
					break

				if data:
					print("received data %s" % repr(data))
					cli.send(1, data[12:])

			cli.close()

		server.stop()

