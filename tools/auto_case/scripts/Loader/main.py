# coding=utf-8
'''
usage : autocase -u username -p password -h=ip:port -m module_name -i case_index --report_file file --report_level 1 --debug rfd wfd
'''
import os
import os.path
import sys
import getopt
import version

sys.path.append('../Case')
sys.path.append('../Module')

import restrict
import network

from logger import setup_logger
from config import option
from Session import session
# 分析参数
opts, args = getopt.getopt( sys.argv[1:], "u:p:m:h:i:", ["report_file=", "report_level=", "debug", "gui"] )

# 脚本的版本
version = None

print( "current workspace directory is %s" % os.getcwd() )

# 设置配置
for opt, arg in opts:
	print( "opt = %s, arg = %s" % (opt, arg) )
	# 账号
	if opt == "-u":
		option.username = arg
	# 密码
	elif opt == "-p":
		option.password = arg
	# 模块名
	elif opt == "-m":
		option.import_name = arg
	# 地址
	elif opt == "-h":
		parts = arg.split( ':' )
		option.host = parts[0]
		option.port = int(parts[1])
	elif opt == "-i":
		option.index = int(arg)
	elif opt == "--report_file":
		option.report_file = arg
	elif opt == "--report_level":
		option.report_level = int(arg)
	elif opt == "--debug":
		option.debug = True
	elif opt == "--gui":
		option.gui = True
	else:
		print( "opt = %s, arg = %s not found" % (opt, arg) )

if option.gui:
	from tkinter import *
	from tkinter.ttk import *

	class Application(Frame):
		def __init__(self,root):
			super().__init__(root)

			frame = self.winfo_toplevel()
			assert(root==frame)

			# print( inspect.getargspec(self.grid) )
			self.grid(row=0, column=0)
			self._createWidgets(frame)

		def _createWidgets(self,frame):
			option.load()

			lb = Label(frame, text="HOST:")
			lb.grid(row=0, column=0, padx = 3, pady = 3, sticky=E)
			self.hostVar = StringVar(value=option.host)
			self.host = Entry(frame, textvariable=self.hostVar)
			self.host.grid(row=0,column=1, padx = 3, pady = 3, sticky=NSEW)

			lb = Label(frame, text="PORT:")
			lb.grid(row=1, column=0, padx = 3, pady = 3, sticky=E)
			self.portVar = IntVar(value=option.port)
			self.port = Entry(frame, textvariable=self.portVar)
			self.port.grid(row=1,column=1, padx = 3, pady = 3, sticky=NSEW)

			Cases = []
			for path in os.listdir('../Case/'):
				if not os.path.isdir('../Case/%s' % (path)):
					continue

				Cases.append(path)

			lb = Label(frame, text="CASE:")
			lb.grid(row=2, column=0, padx = 3, pady = 3, sticky=E)

			self.caseVar = StringVar(value=option.import_name)
			self.case = Combobox(frame, values=Cases, textvariable=self.caseVar)
			self.case.grid(row=2,column=1, padx = 3, pady = 3, sticky=NSEW)

			#user's name and password
			lb = Label(frame, text="USERNAME:")
			lb.grid(row=3, column=0, padx = 3, pady = 3, sticky=E)
			self.usernameVar = StringVar(value=option.username)
			self.username = Entry(frame, textvariable=self.usernameVar)
			self.username.grid(row=3,column=1, padx = 3, pady = 3, sticky=NSEW)

			lb = Label(frame, text="PASSWORD:")
			lb.grid(row=4, column=0, padx = 3, pady = 3, sticky=E)
			self.passwordVar = StringVar(value=option.password)
			self.password = Entry(frame, textvariable=self.passwordVar)
			self.password.grid(row=4,column=1, padx = 3, pady = 3, sticky=NSEW)

			#report file
			lb = Label(frame, text="REPORT:")
			lb.grid(row=5, column=0, padx = 3, pady = 3, sticky=E)
			self.reportVar = StringVar(value=option.report_file)
			self.report = Entry(frame, text=option.report_file, textvariable=self.reportVar)
			self.report.grid(row=5,column=1, padx = 3, pady = 3, sticky=NSEW)

			button = Button(frame, text='OK', width=40)
			button.grid(row=7, column=1, padx=5, pady=5, sticky=E)
			button.bind( '<Button-1>', self._ok )

			frame.grid_rowconfigure(0, weight=0)
			frame.grid_rowconfigure(1, weight=0)
			frame.grid_columnconfigure(0, minsize=60, weight=0)
			frame.grid_columnconfigure(1, weight=4)

			frame.resizable(False, False)

		def _ok(self, event):
			option.import_name = self.case.get()
			option.username = self.username.get()
			option.password = self.password.get()
			option.report_file = self.report.get()
			option.host = self.host.get()
			option.port = int(self.port.get())
			
			option.save()

			self.winfo_toplevel().withdraw()
			self.quit()

	root = Tk()
	root.title('Auto Case GUI')
	root.geometry('300x400')

	app = Application(root)
	root.mainloop()

# 模块名必须配置
if option.import_name is None:
	print( "module_name not special." )
	print( __doc__ )
	sys.exit(-1)

# 配置日志，生成日志文件
setup_logger()

# 注册全局函数
__builtins__['call'] = restrict.call

# sys.meta_path.insert(0, MetaPathFinder())
# 动态载入测试模块
robot = __import__( option.import_name )

command("tips 'start test username=%s, password=%s, module=%s, address=%s:%d'" % (
	option.username,
	option.password,
	option.import_name,
	option.host,
	option.port)) # yapf: disable

account.info('this\'s %s here.' % __name__)
account.info('start test case...')

if option.index >= len(robot.package):
	ERR( "select case is not exist." )
	sys.exit(-1)

cli = network.client( robot.package[option.index], session.role, robot.hook )

# main loop
account.info('-------------------------------------')
account.info("connect server %s:%d" % (option.host, option.port))

cli.session.setup_report(option.report_file, option.report_level)
cli.session.report( Log=None if 'comment' not in cli.case else cli.case['comment'])

restrict.trigger( cli, "start" )

# 主逻辑循环
while cli.restrict:
	try:
		# 超时检查
		cli.restrict.check_timeout()

		ret = cli.recv()
		if ret == -1:
			break
		elif ret == -2:
			report(Log='reconnect')
			account.error("recv error reconnect now.")
			cli.reconnect()
			continue
		else:
			time.sleep(0.01)

		if cli.restrict.node is None:
			break
			
	except restrict.exception as e:
		# 捕获到异常则认为失败，直接退出
		account.error( '%s - %s' % (e.value(), e.message()))
		break

account.info('-------------------------------------')
account.info('test end...')

cli.close()
