from tkinter import *
from tkinter.ttk import *

class ButtonDialog:
    def __init__(self, parent, title=None):
        self.__toplevel = Toplevel(parent)
        if title:
            self.__toplevel.title(title)

        self.__parent = parent
        self.__result = None

    def show(self, modaless=False):
        self.__body = Frame(self.__toplevel)
        self._initdialog(self.__body)

        self.__buttons = Frame(self.__toplevel)
        self._initbuttons(self.__buttons)

        self.__toplevel.rowconfigure(0, weight=1)
        self.__toplevel.columnconfigure(0, weight=1)

        self.__body.grid(row=0, column=0, padx=5, pady=5, sticky=NSEW)
        self.__buttons.grid(row=1, column=0, padx=5, pady=5, sticky=NSEW)

        self.__toplevel.wait_window()
        return self.__result

    def _initdialog(self, master):
        pass

    def _initbuttons(self, master):
        btn = Button(master, text='Cancel', width=10, command=self._cancel )
        btn.pack(side=RIGHT, padx=5, pady=5)

        btn = Button(master, text='OK', width=10, command=self._ok, default=ACTIVE )
        btn.pack(side=RIGHT, padx=5, pady=5)

        self.__toplevel.bind('<Return>', self._ok)
        self.__toplevel.bind('<Escape>', self._cancel)

    def _cancel(self, event=None):
        self.__parent.focus_set()
        self.__toplevel.withdraw()
        self.__toplevel.update_idletasks()
        self.__toplevel.destroy()

        self.__result = 'cancel'

        
    def _ok(self, event=None):
        self.__parent.focus_set()
        self.__toplevel.withdraw()
        self.__toplevel.update_idletasks()

        self.apply(event)

        self.__toplevel.destroy()
        self.__result = 'ok'

    def apply(self, event=None):
        pass


