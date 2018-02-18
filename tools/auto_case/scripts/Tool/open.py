import os, sys
import inspect

from tkinter import *
from tkinter.ttk import *
from widgets import *
from message import *
import restrict

class OpenCaseDialog(ButtonDialog):
	def __init__(self, master, title=None):
		super().__init__(master, title)

		self.__select = None

	def _initdialog(self, master):
		master.rowconfigure(0, weight=1)
		master.columnconfigure(0, weight=1)

		self.__caselist = Listbox(master, selectmode=SINGLE)
		self.__caselist.pack(side=LEFT, fill=BOTH, expand=1)

		for path in os.listdir('../Case/'):
			if not os.path.isdir('../Case/%s' % (path)):
				continue

			self.__caselist.insert(END, path)

		scroll = Scrollbar(master, orient=VERTICAL, command=self.__caselist.yview)
		scroll.pack(side=RIGHT, fill=Y )

		self.__caselist.config(yscrollcommand = scroll.set)
		self.__caselist.bind( '<Double-1>', self._ok )

	def apply(self, event=None):
		self.__select = self.__caselist.selection_get()

	def result(self):
		return self.__select

class SaveCaseDialog(ButtonDialog):
	def __init__(self, master, cases, title=None):
		super().__init__(master, title)

		self._cases = [(name, node, tk.BooleanVar()) for name, node in cases]

	def _initdialog(self, master):
		for k, n, v in self._cases:
			case = Checkbutton(master, text=k, variable=v, onvalue=True, offvalue=False)
			case.pack(fill=X)
	
	def _checked(self, event=None):
		print(self._cases)
		pass

	def apply(self, event=None):
		pass

	def result(self):
		return [(k, n) for k, n, v in self._cases if v.get()]
