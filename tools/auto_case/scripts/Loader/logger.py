import os
import time
import logging
import logging.config

from config import option
from utility import make_path

class debug_filter(logging.Filter):

	def filter(self, record):
		return record.levelno == logging.DEBUG

class infomation_filter(logging.Filter):

	def filter(self, record):
		return record.levelno >= logging.INFO

def setup_logger():
	if not os.path.exists('logs'):
		os.mkdir('logs')

	log_path = 'logs/%s/%s' % (option.import_name, option.username)
	log_file = 'logs/%s/%s/%s.log' % (option.import_name, option.username, time.strftime("%Y%m%d_%H%M%S"))

	make_path(log_path)

	log_conf = {
		'version':
			1,
		'disable_existing_loggers':
			False,
		'formatters': {
			'debug': {
				'format': '%(levelname)7s: %(message)s (%(filename)s:%(lineno)d)',
			},
			'infomation': {
				'format': '%(levelname)7s: %(message)s',
			},
			'file': {
				'format': '%(asctime)s [%(levelname)7s] : %(message)s (%(filename)s:%(lineno)d)',
				'datefmt': '%Y-%m-%d %H:%M:%S'
			},
		},
		'filters': {
			'debug': {
				'()': 'logger.debug_filter',
			},
			'infomation': {
				'()': 'logger.infomation_filter',
			}
		},
		'handlers': {
			'framework': {
				'class': 'logging.handlers.RotatingFileHandler',
				'level': 'WARNING',
				'formatter': 'file',
				'filename': 'logs/loader.log',
				'maxBytes': 10 * 1024 * 1024,
				'backupCount': 10
			},
			'account': {
				'class': 'logging.FileHandler',
				'level': 'DEBUG',
				'formatter': 'file',
				'filename': log_file,
			},
			'console debug': {
				'class': 'logging.StreamHandler',
				'level': 'DEBUG',
				'formatter': 'debug',
				'filters': ['debug'],
			},
			'console infomation': {
				'class': 'logging.StreamHandler',
				'level': 'INFO',
				'formatter': 'infomation',
				'filters': ['infomation'],
			}
		},
		'loggers': {
			'root': {
				'handlers': ['console debug', 'console infomation', 'framework'],
				'propagate': False,
			},
			'account': {
				'handlers': ['console debug', 'console infomation', 'account'],
				'propagate': False,
			},
		},
		'root': {
			'level': 'DEBUG',
		},
	}

	logging.config.dictConfig(log_conf)

	global_dict = __builtins__

	global_dict["DBG"] = logging.getLogger("root").debug
	global_dict["INF"] = logging.getLogger("root").info
	global_dict["WRN"] = logging.getLogger("root").warning
	global_dict["ERR"] = logging.getLogger("root").error
	global_dict["CRI"] = logging.getLogger("root").critical
	global_dict["account"] = logging.getLogger("account")


if __name__ == '__main__':
	setup_logger()

	DBG("DBG")
	WRN("WRN")

	account.debug('debug')
