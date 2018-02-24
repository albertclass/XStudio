import os
from tkinter import *
from tkinter.ttk import *

from config import option

class Application(Frame):
	def __init__(self, wnd = Tk()):
		super().__init__(wnd)

		self.__wnd = wnd
		self.__wnd.title('Auto Case GUI')
		self.__state = 0

		frame = self.winfo_toplevel()

		# print( inspect.getargspec(self.grid) )
		self.pack(anchor=CENTER, expand=1)
		self._createWidgets(frame)

	def _createWidgets(self,frame):
		option.load()

		lf = LabelFrame(frame, text='Configuration' )
		lf.pack(fill=X, side=TOP, padx=5, pady=5)

		lb = Label(lf, text="HOST:")
		lb.grid(row=0, column=0, padx = 3, pady = 3, sticky=E)
		self.hostVar = StringVar(value=option.host)
		self.host = Entry(lf, textvariable=self.hostVar)
		self.host.grid(row=0,column=1, padx = 3, pady = 3, sticky=NSEW)

		lb = Label(lf, text="PORT:")
		lb.grid(row=1, column=0, padx = 3, pady = 3, sticky=E)
		self.portVar = IntVar(value=option.port)
		self.port = Entry(lf, textvariable=self.portVar)
		self.port.grid(row=1,column=1, padx = 3, pady = 3, sticky=NSEW)

		Cases = []
		for path in os.listdir('../Case/'):
			if not os.path.isdir('../Case/%s' % (path)):
				continue

			Cases.append(path)

		lb = Label(lf, text="CASE:")
		lb.grid(row=2, column=0, padx = 3, pady = 3, sticky=E)

		self.caseVar = StringVar(value=option.import_name)
		self.case = Combobox(lf, values=Cases, textvariable=self.caseVar)
		self.case.grid(row=2, column=1, padx = 3, pady = 3, sticky=NSEW)

		#user's name and password
		lb = Label(lf, text="USERNAME:")
		lb.grid(row=3, column=0, padx = 3, pady = 3, sticky=E)
		self.usernameVar = StringVar(value=option.username)
		self.username = Entry(lf, textvariable=self.usernameVar)
		self.username.grid(row=3,column=1, padx = 3, pady = 3, sticky=NSEW)

		lb = Label(lf, text="PASSWORD:")
		lb.grid(row=4, column=0, padx = 3, pady = 3, sticky=E)
		self.passwordVar = StringVar(value=option.password)
		self.password = Entry(lf, textvariable=self.passwordVar)
		self.password.grid(row=4,column=1, padx = 3, pady = 3, sticky=NSEW)

		#report file
		lb = Label(lf, text="REPORT:")
		lb.grid(row=5, column=0, padx = 3, pady = 3, sticky=E)
		self.reportVar = StringVar(value=option.report_file)
		self.report = Entry(lf, text=option.report_file, textvariable=self.reportVar)
		self.report.grid(row=5,column=1, padx=3, pady=3, sticky=NSEW)

		#debug mode
		lb = Label(lf, text="DEBUG:")
		lb.grid(row=6, column=0, padx = 3, pady = 3, sticky=E)
		self.debugVar = BooleanVar(value=option.debug)
		self.debug = Checkbutton(lf, onvalue=True, offvalue=False, variable=self.debugVar)
		self.debug.grid(row=6, column=1, padx=3, pady=3, sticky=NSEW)

		lf.grid_columnconfigure(0, minsize=20, weight=0)
		lf.grid_columnconfigure(1, minsize=30, weight=1)

		fr = Frame(frame)
		button = Button(fr, text='Cancel')
		button.pack(side=RIGHT)
		button.bind( '<Button-1>', self._cancel )

		button = Button(fr, text='OK')
		button.pack(side=RIGHT)
		button.bind( '<Button-1>', self._ok )

		fr.pack(fill=X, side=BOTTOM, padx=10, pady=10)

		frame.resizable(False, False)

	def _ok(self, event):
		self.__state = 1
		option.import_name = self.case.get()
		option.username = self.username.get()
		option.password = self.password.get()
		option.report_file = self.report.get()
		option.debug = self.debugVar.get()
		option.host = self.host.get()
		option.port = int(self.port.get())
		
		option.save()
		self.quit()

	def _cancel(self, event):
		self.__state = 0
		self.quit()

	def getState(self):
		return self.__state

	def show(self):
		self.update_idletasks()
		w = self.winfo_screenwidth()
		h = self.winfo_screenheight()
		s = (260, 280)
		x = w/2 - s[0]/2
		y = h/2 - s[1]/2

		self.__wnd.geometry("%dx%d+%d+%d" % (s + (x, y)))
		self.__wnd.mainloop()
		self.winfo_toplevel().withdraw()

app = Application()

