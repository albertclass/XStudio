import os, os.path
import time
import datetime
import csv
import utility

from config import option

report_header = ['Package', 'Case', 'Step', 'Log']

class Report:
	def __init__(self):
		self.fd = None
		self.wd = None

	def setup( self, file=None, level=0, header=report_header ):
		if len(file) == 0: return False

		path = os.path.dirname(file)
		name = os.path.basename(file)

		utility.make_path(path)

		# csv report
		fd = self.fd = open(file, 'w+')
		if not fd:
			return False

		wd = self.wd = csv.writer(self.fd)
		if not wd:
			return False
		
		header.insert( 0, 'Time' )
		wd = self.wd = csv.DictWriter(fd, header)
		if not wd:
			return False

		wd.writeheader()
		return True

	def close(self):
		if self.fd:
			self.fd.close()

	def record_some(self, **kw):
		if self.fd and self.wd:
			kw['Time'] = datetime.datetime.now().strftime('%H:%M:%S.%f')
			self.wd.writerow(kw)
			self.fd.flush()

if __name__ == '__main__':
	option.report_file = 'report.csv'
	option.report_level = 1
	rpt = Report()
	rpt.setup( option.report_file, option.report_level )
	
	rpt.record_some( Package ='Login' )
	rpt.record_some( Case ='UserLogin1', Step="Login" )
	rpt.record_some( Log='recv msg=1003' )
	rpt.close()
	print('test finished')
