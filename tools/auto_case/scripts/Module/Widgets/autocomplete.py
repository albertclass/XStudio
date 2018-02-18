import tkinter as tk
import tkinter.ttk as ttk
import tkinter.font

class SuggestEntry(tk.Entry):
	def __init__(self, master=None, values=None, **kw):
		super().__init__(master, **kw)
		self._toplevel = tk.Toplevel(master)
		self._toplevel.overrideredirect(1)
		self._toplevel.withdraw()

		self._listbox = tk.Listbox(self._toplevel, selectmode=tk.BROWSE)
		self._listbox.pack(fill=tk.BOTH, expand=1)

		self._suggest_list = sorted(values, key=str.lower)
		self._hits=[]
		
		self._hit_index=0
		self.position=0

		self.bind('<KeyRelease>', self.__onKeyRelease)
		self.bind('<FocusOut>', self.__onFocusOut)

	def autocomplete(self, delta=0):
		if delta:
			self.delete(self.position, tk.END)
		else:
			self.position=len(self.get())

		_hits=[]
		for element in self._suggest_list:
			if element.lower().find(self.get().lower()) != -1:
				_hits.append(element)

		self._hits = _hits
		'''
		if _hits != self._hits:
			self._hit_index=0
			self._hits=_hits

		if _hits == self._hits and self._hits:
			self._hit_index=(self._hit_index+delta) % len(self._hits)

		if self._hits:
			self.delete(0, tk.END)
			self.insert(0, self._hits[self._hit_index])
			self.select_range(self.position, tk.END)
		'''

	def __updateListbox(self):
		maxl = max(self._suggest_list, key = lambda v : len(v) )
		f1 = tk.font.nametofont(str(self._listbox.cget('font')))
		w1 = f1.measure(maxl)
		h1 = f1.metrics('linespace')

		f2 = tk.font.nametofont(str(self.cget('font')))
		w2 = f2.measure(self.get())
		h2 = f2.metrics('linespace')

		x, y, w, h = self.winfo_rootx(), self.winfo_rooty(), self.winfo_width(), self.winfo_height()
		self._toplevel.geometry('%dx%d+%d+%d' % (w1, h1*10, x+w2, y+h))
		self._toplevel.lift(aboveThis=self.winfo_toplevel())
		self._toplevel.deiconify()
		self._listbox.delete(0, tk.END)
		self._listbox.insert(tk.END, *self._hits)


	def __onFocusOut(self, event):
		self._toplevel.withdraw()

	def __onKeyRelease(self, event):
		if event.keysym == 'BackSpace':
			self.delete(self.index(tk.INSERT), tk.END)
			self.position = self.index(tk.END)

			self.autocomplete()
			self.__updateListbox()

		elif event.keysym == 'Left':
			if self.position < self.index(tk.END):
				self.delete(self.position, tk.END)
			else:
				self.position = self.position - 1
				self.delete(self.position, tk.END)

		elif event.keysym == 'Right':
			self.position = self.index(tk.END)

		elif event.keysym == 'Up':
			self._hit_index = self._hit_index - 1
			while self._hit_index < 0:
				self._hit_index = self._hit_index + self._listbox.size()

			self._listbox.selection_clear(0,tk.END)
			self._listbox.selection_set(self._hit_index)

		elif event.keysym == 'Down':
			self._hit_index = self._hit_index + 1
			while self._hit_index >= self._listbox.size():
				self._hit_index = self._hit_index - self._listbox.size()

			self._listbox.selection_clear(0,tk.END)
			self._listbox.selection_set(self._hit_index)
		
		elif event.keysym == 'Escape':
			if self._toplevel.state() == 'normal':
				self._toplevel.withdraw()
			else:
				self.master.focus_set()
			

		elif event.keysym == 'Return':
			if self._toplevel.state() == 'normal':
				sels = self._listbox.curselection()
				if len(sels) > 0:
					item = self._listbox.get(sels)
				elif self._listbox.size() > 0:
					item = self._listbox.get(0)
				else:
					item = None

				if item :
					self.delete(0, tk.END)
					self.insert(0, item)
					self.icursor(tk.END)

				self._toplevel.withdraw()
			else:
				self.master.focus_set()

		elif len(event.keysym) == 1:
			self.autocomplete()
			self.__updateListbox()
	
class ComboboxEx(ttk.Combobox):

	def __init__(self, master=None, **kw):
		super().__init__(master, **kw)
		self._completion_list = sorted(kw['values'], key=str.lower)
		self._hits=[]
		
		self._hit_index=0
		self.position=0

		self.bind('<KeyRelease>', self.__onKeyRelease)
		self.bind('<Configure>', self.__configure)

	def autocomplete(self, delta=0):
		if delta:
			self.delete(self.position, tk.END)
		else:
			self.position=len(self.get())

		_hits=[]
		for element in self._completion_list:
			if element.lower().startswith(self.get().lower()):
				_hits.append(element)

		if _hits != self._hits:
			self._hit_index=0
			self._hits=_hits

		if _hits == self._hits and self._hits:
			self._hit_index=(self._hit_index+delta) % len(self._hits)

		if self._hits:
			self.delete(0, tk.END)
			self.insert(0, self._hits[self._hit_index])
			self.select_range(self.position, tk.END)

		self['values'] = self._hits

	def __onKeyRelease(self, event):
		# self.event_generate('<Down>')

		if event.keysym == 'BackSpace':
			self.delete(self.index(tk.INSERT), tk.END)
			self.position = self.index(tk.END)

		if event.keysym == 'Left':
			if self.position < self.index(tk.END):
				self.delete(self.position, tk.END)
			else:
				self.position = self.position - 1
				self.delete(self.position, tk.END)

		if event.keysym == 'Right':
			self.position = self.index(tk.END)

		if len(event.keysym) == 1:
			self.autocomplete()

	def __configure(self, event):
		maxl = max(self._completion_list, key = lambda v : len(v) )
		f = tk.font.nametofont(str(event.widget.cget('font')))
		w = f.measure(maxl) - event.width
		css = ttk.Style()
		css.configure('TCombobox', postoffset=(0,0,w,0))


if __name__ == '__main__':
	ll = ('albert', 'sophia', 'ccc', 'acc', 'abc', 'so', 'Acornccccccccccccccccccccccccccccc')

	root = tk.Tk(className='autocomplete demo')
	combo = ComboboxEx(root, values=ll)
	combo.pack()

	entry = SuggestEntry(root, values=ll)
	entry.pack()

	root.mainloop()