import os
import configparser

class option:
	import_name = 'login'
	username = 'robot0000'
	password = '123456789'
	debug = False
	report_file = None
	report_level = 4
	host = '127.0.0.1'
	port = 50002
	pipe = [-1, -1]
	index = 0

	def save():
		cf = configparser.ConfigParser()
		cf.add_section("option")

		cf.set("option", "host", option.host)
		cf.set("option", "port", str(option.port))
		cf.set("option", "import_name", option.import_name)
		cf.set("option", "username", option.username)
		cf.set("option", "password", option.password)
		cf.set("option", "debug", str(option.debug))
		cf.set("option", "report_file", option.report_file)
		cf.set("option", "report_level", str(option.report_level))

		with open("config.conf", mode="w+") as fp:
			cf.write( fp )
			fp.flush()

	def load():
		cf = configparser.ConfigParser()
		if cf.read( 'config.conf' ) :
			option.host = cf.get("option", "host")
			option.port = cf.getint("option", "port")
			option.import_name = cf.get("option", "import_name")
			option.username = cf.get("option", "username")
			option.password = cf.get("option", "password")
			option.debug = cf.getboolean("option", "debug")
			option.report_file = cf.get("option", "report_file")
			option.report_level = cf.getint("option", "report_level")

class gateinfo:
	addr = ''
	port = 0
	token = ''

def command(line):
	pass
	#os.write(option.pipe[1], line.encode() + b'\0')

__builtins__["command"] = command
