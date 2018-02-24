# coding=utf-8
'''
usage : autocase -u username -p password -h=ip:port -m module_name -i case_index --report_file file --report_level 1 --debug
'''
import os
import os.path
import sys
import time
import getopt
import version

sys.path.append('../Case')
sys.path.append('../Module')

import restrict
import client

from logger import setup_logger
from config import option
from Session import session
from gui import app

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
	app.show()

# 模块名必须配置
if option.import_name is None:
	print( "module_name not special." )
	print( __doc__ )
	sys.exit(-1)

# 配置日志，生成日志文件
setup_logger()

# 注册全局函数
__builtins__['assign'] = restrict.assign
__builtins__['invoke'] = restrict.invoke
__builtins__['var'] = restrict.var

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

if option.index >= len(robot.packages):
	ERR( "select case is not exist." )
	sys.exit(-1)

cli = client.Client( robot.packages[option.index], session.role, robot.hook )

# main loop
account.info('-------------------------------------')
account.info("connect server %s:%d" % (option.host, option.port))

cli.setup_report(option.report_file, option.report_level)
cli.report( Log=None if 'comment' not in cli.package else cli.package['comment'])

try:
	restrict.trigger( cli, "start" )
except restrict.exception as e:
	# 捕获到异常则认为失败，直接退出
	account.error( '%s - %s' % (e.value(), e.message()))

# 主逻辑循环
while cli.restrict:
	try:
		if cli.loop():
			time.sleep(0.01)

		if cli.throw is None:
			break
			
	except restrict.exception as e:
		# 捕获到异常则认为失败，直接退出
		account.error( '%s - %s (%s:%d)%s' % (e.value(), e.message(), e.file(), e.line(), e.trackback()))
		break

account.info('-------------------------------------')
account.info('test complate. the last stage is %s' % (cli.test))