import os
import sys
sys.path.append('../Loader')
sys.path.append('../Case')

from tkinter import *
from tkinter.ttk import *
from tkinter.filedialog import *
from inspect import *
from itertools import zip_longest

from utility import *
from functools import partial
from widgets import *
from template import *
from monitor import *

from message import *
from parse import *
from open import *

class Application(Frame):
	def __init__(self,root):
		super().__init__(root)

		self._casename = None
		self._filemonitor = FileMonitor()
		self._filemonitor.bind('created', self._onMonitorCreatedFile)
		self._filemonitor.bind('deleted', self._onMonitorDeletedFile)
		self._filemonitor.bind('modified', self._onMonitorModifiedFile)
		self._filemonitor.bind('moved', self._onMonitorMovedFile)

		mainFrame = self.winfo_toplevel()
		assert(root==mainFrame)
		mainFrame.rowconfigure(0, weight=1)
		mainFrame.columnconfigure(0, weight=1)

		# print( inspect.getargspec(self.grid) )
		self.grid(row=0, column=0, sticky=NSEW)
		self._createWidgets(mainFrame)

		# builder = CaseBuilder(self.lTree, 'altername', template)
		# builder.build()

	def _createWidgets(self,mainFrame):
		self.menubar = Menu(mainFrame, tearoff=0)
		self.fileMenu = Menu(self.menubar, tearoff=0)
		self.fileMenu.add_command(label='Open Case', underline=6, command=self._onOpenCase)
		self.fileMenu.add_command(label='Save Case', underline=0, command=self._onSaveCase)

		self.fileMenu.add_separator()
		self.fileMenu.add_command(label='Exit', command=self._onExit)

		self.menubar.add_cascade(label='File', menu=self.fileMenu)
		self.menubar.add_command(label='About', command=self._About)

		mainFrame.config(menu=self.menubar)

		w = PanedWindow(self, orient=VERTICAL)
		w.pack(fill=BOTH, expand=1)

		self.lPane = Frame(w)
		self.lPane.grid_rowconfigure(0, weight=1)
		self.lPane.grid_columnconfigure(0, weight=1)

		self.lTree = TreeviewEx(self.lPane)
		self.lTree.grid(row=0, column=0, sticky=NSEW)

		scroll_y = Scrollbar(self.lPane, orient=VERTICAL, command=self.lTree.yview )
		scroll_y.grid(row=0, column=1, sticky=NS)

		scroll_x = Scrollbar(self.lPane, orient=HORIZONTAL, command=self.lTree.xview )
		scroll_x.grid(row=1, column=0, sticky=EW)

		self.lTree['yscrollcommand'] = scroll_y.set
		self.lTree['xscrollcommand'] = scroll_x.set

		# 左侧窗口配置
		self.lTree['columns'] = ('1', '2', '3')
		self.lTree.column('#0', stretch=True, width=200)
		self.lTree.column('1', stretch=True, width=280)
		self.lTree.column('2', stretch=False, width=80)
		self.lTree.column('3', stretch=False, width=80)

		self.lTree.heading('#0', text='node')
		self.lTree.heading('1', text='value')
		self.lTree.heading('2', text='name')
		self.lTree.heading('3', text='vtype')

		self.lTree.bind( '<Button-3>', self._onTreeRightButtonPress, add='+' )
		self.lTree.bind( '<<TreeviewCellEdited>>', self._onTreeCellEdited )

		idname = sorted( ids.names.items(), key=lambda item : item[0] )
		idnums = ['ids.' + v for k, v in idname]

		self._inplace_suggest_entry = SuggestEntry(self.lTree, values=idnums, textvariable=tk.StringVar())
		w.add(self.lPane)

		# 右侧窗口配置
		self.notebook = notebook = self.rPane = Notebook(w)

		# 状态转换图
		drawview = Frame(notebook)
		drawview.grid_rowconfigure(0, weight=1)
		drawview.grid_columnconfigure(0, weight=1)

		self.rCanv = Canvas( drawview, bg='#FFF', scrollregion=(0,0,1080,1024))
		self.rCanv.grid(row=0, column=0, sticky=NSEW)

		scroll_y = Scrollbar(drawview, orient=VERTICAL, command=self.rCanv.yview )
		scroll_y.grid(row=0, column=1, sticky=NS)

		scroll_x = Scrollbar(drawview, orient=HORIZONTAL, command=self.rCanv.xview )
		scroll_x.grid(row=1, column=0, sticky=EW)

		self.rCanv['yscrollcommand'] = scroll_y.set
		self.rCanv['xscrollcommand'] = scroll_x.set

		notebook.add(drawview, text='state machine')
		w.add(notebook)

		# label = Label(self, text="hello world!", background='red')
		# label.grid(row=0, column=0)
	def _getItemPath(self, iid):
		if not iid: return []

		item = self.lTree.item(iid)
		tags = item['tags']
		type = tags[0]

		path = self._getItemPath(self.lTree.parent(iid))
		path.append(type)
		return path

	def _getItemConf(self, iid):
		return get_template_node(self._getItemPath(iid))

	def _onTreeRightButtonPress(self, event):
		iid = self.lTree.identify('item', event.x, event.y)
		# 设置右键点击的节点为选中状态
		if iid:
			self.lTree.selection_set(iid)

		menu = Menu(self.lTree, tearoff=0)

		# 获取树路径对应的配置项
		conf = self._getItemConf(iid)

		ttype = self.lTree.set(iid, '3') if iid else None 

		children = (conf['children'] if 'children' in conf else []) if conf else [template]
		for c in children:
			if 'menu' not in c:
				continue
			
			if 'text' not in c:
				continue

			if 'tags' not in c:
				continue

			if 'mode' not in c:
				continue

			if 'type' not in c:
				continue

			mode = c['mode']
			if mode == 'once':
				children = self.lTree.get_children(iid)
			else:
				children = []

			for c_iid in children:
				# 查找是否该节点已被创建
				if self.lTree.tag_has(c['tags'], c_iid ):
					break
			else:
				# 没有节点重复限制，增加菜单项

				# 先查看节点的类型限制
				if ttype is None or ttype in ((str(x) for x in flatitem(c['when'])) if 'when' in c else [ttype]):
					if inspect.isclass(c['type']):
						# 单一类型的情况，直接增加菜单项
						menu.add_command(label=c['menu'], command=partial(self._onTreeAddChild, node=iid, conf=c, vtype=c['type']))
					else:
						# 多种类型的情况，按类型增加菜单项
						for vtype in flatitem(c['type']):
							menu.add_command(label=c['menu'] + ' - ' + vtype.__name__, command=partial(self._onTreeAddChild, node=iid, conf=c, vtype=vtype))
		
		if conf and 'delete' in conf and conf['delete']:
			# 查看是否需要添加删除菜单项
			if menu.index(END) is not None:
				# 前面添加了菜单项的情况下，添加一个分割条
				menu.add_separator()

			menu.add_command(label='删除节点', command=partial(self._onTreeDelChild, node=iid))

		if menu.index(END) is not None:
			# 没有菜单项则不弹出菜单
			menu.post(event.x_root, event.y_root)

	def _onTreeAddChild(self, node, conf, vtype):
		'''
		添加节点，如果添加Test节点会自动生成子节点
		'''
		sort = conf['sort'] if 'sort' in conf else 'end' 
		name = conf['name'] if 'name' in conf else '' 

		tree = self.lTree
		# 插入节点
		item = tree.insert(node, sort, open=True, text=conf['text'] or name, tags=conf['tags'], values=['', name, vtype])
		when = conf['when'] if 'when' in conf else vtype

		while 'edit' in conf:
			col, flag, options = [x for x, _ in zip_longest(conf['edit'], range(3))]
		
			if flag == 'text':
				tree.inplace_entry(col, item)
				break
			
			if flag == 'spin':
				if options and 'args' in options:
					tree.inplace_spinbox(col, item, *options['args'])
				else:
					tree.inplace_spinbox(col, item, 0, 100, 1)
				break

			if flag == 'message':
				widget = SuggestEntry(self.lTree, values=idnums, textvariable=tk.StringVar())
				tree.inplace_custom(col, item, widget)
				break

			if flag == 'function':
				tree.inplace_entry(col, item)
				break
			
			break

		if 'children' in conf:
			children = (x for x in conf['children'] if 'make' in x and x['make'] and vtype in flatitem(when))
			for child in children:
				# 默认创建的节点类型一定是一个确定的类型
				self._onTreeAddChild(item, child, next(flatitem(child['type'])))
	
	def _onTreeDelChild(self, node):
		self.lTree.delete( node )
		pass

	def _onTreeCellEdited(self, event=None):
		tree = self.lTree
		col, iid = tree.get_event_info()
		conf = self._getItemConf(iid)

		if not conf:
			return

		if 'edit' not in conf:
			return

		edit, flag, options = [x for x, _ in zip_longest(conf['edit'], range(3))]
		value = tree.item(iid, 'text') if col == '#0' else tree.set(iid, column=edit)
		if options and 'update' in options:
			for col in flatitem(options['update']):
				if col == '#0':
					tree.item(iid, text=value)
				else:
					tree.set(iid, col, value)
					

	def _Open(self, filename):
		if filename:
			editor = Editor(self.notebook)
			self.notebook.add(editor, text='noname')
			editor.bind('<<reload>>', self._onFileChanged)
			editor.bind('<<save>>', self._onFileChanged)
			editor.bind('<<open>>', self._onFileChanged)

			editor.open(filename, '..\\Case')

	def _onOpenCase(self):
		if self._casename:
			# 关闭已打开的Case
			tabs = [self.nametowidget(tab) for tab in self.notebook.tabs()]
			for tab in tabs:
				if isinstance(tab, Editor):
					tab.close()

		dlg = OpenCaseDialog(self, 'Open Case ...')
		if dlg.show(True) == 'ok':
			self._casename = dlg.result()

			directory = os.path.abspath(os.getcwd() + '\\..\\Case\\%s' % self._casename)
			# 获取目录下的文件
			filelist = [file for file in os.listdir(directory)]

			for file in filelist:
				filename = '\\'.join((directory, file))
				if os.path.isdir(filename):
					# 略过目录
					continue
				
				if os.path.splitext(file)[1] != '.py':
					# 略过非脚本
					continue
					
				# 生成页签
				self._Open(filename)

			if self.fileMenu.index(tk.END) == 3:
				self.fileMenu.insert_separator(2)
				self.fileMenu.insert_command(3, label='New File', accelerator='Ctrl + N', command=self._onNew)
				self.bind_all('<Control-KeyPress-n>', self._onNew)
				self.fileMenu.insert_command(4, label='Open File', accelerator='Ctrl + O', command=self._onOpen)
				self.bind_all('<Control-KeyPress-o>', self._onOpen)
				self.fileMenu.insert_command(5, label='Save File', accelerator='Ctrl + S', command=self._onSave)
				self.bind_all('<Control-KeyPress-s>', self._onSave)
				self.fileMenu.insert_command(6, label='Save All Files', accelerator='Ctrl + Shift + S', command=self._onSaveAll)
				self.bind_all('<Control-Shift-KeyPress-s>', self._onSaveAll)
				self.fileMenu.insert_command(7, label='Close File', accelerator='Ctrl + W', command=self._onClose)
				self.bind_all('<Control-KeyPress-w>', self._onClose)

			self._filemonitor.reset(directory, True)
	
	
	def _onSaveCase(self):
		if self._casename:
			tree = self.lTree
			cases = [(tree.item(child, 'text'), child) for child in tree.get_children()]
			dlg = SaveCaseDialog(self, cases, 'Select Case Package.')
			if dlg.show(True) != 'ok':
				return

			children = dlg.result()

			w = CaseParser()
			w.prase(self.lTree, '', template)

			w.write('\ncase_package = [')
			w.indent(+1)
			for idx, child in enumerate(children):
				if idx:
					w.write(',\n%s' % child[0])
				else:
					w.write('\n%s' % child[0])

			w.indent(-1)
			w.write('\n]\n')

			filename = '\\'.join((os.path.abspath('..\\Case'), self._casename, 'case.py'))

			tabs = (self.nametowidget(tab) for tab in self.notebook.tabs())
			for editor in tabs:
				if not isinstance(editor, Editor):
					continue
				
				if editor.filename.lower() != filename.lower():
					continue

				editor.replace(w.dump())

	def _onNew(self, event=None):
		if self._casename:
			editor = Editor(self.notebook)
			self.notebook.add(editor, text='untitled')
			self.notebook.select(editor)

	def _onOpen(self, event=None):
		if self._casename:
			directory = os.path.abspath(os.getcwd() + '\\..\\Case\\%s' % self._casename)
			filename = askopenfilename(
				title='Open',
				initialdir=directory,
				filetypes=(('Python Files', '*.py'), ('All Files', '*.*')))

		self._Open(filename)

	def _onSave(self, event=None):
		if self._casename:
			select = self.notebook.index('current')
			editor = self.nametowidget(self.notebook.tabs()[select])
			if isinstance(editor, Editor) and editor.is_modified():
				editor.save()
		
	def _onSaveAll(self, event=None):
		if self._casename:
			for editor in (self.nametowidget(name) for name in self.notebook.tabs()):
				if isinstance(editor, Editor) and editor.is_modified():
					editor.save()

	def _onClose(self, event=None):
		if self._casename:
			select = self.notebook.index('current')
			editor = self.nametowidget(self.notebook.tabs()[select])
			if isinstance(editor, Editor):
				editor.close()

	def _onFileChanged(self, event):
		editor = event.widget
		base = os.path.basename(editor.filename)
		if base.lower() == 'case.py':
			builder = CaseBuilder(self.lTree, self._casename, template)
			builder.build(True)


	def _onExit(self, event=None):
		self.quit()

	
	def _About(self):
		pass

	def _onMonitorCreatedFile(self, event):
		if not self._casename:
			return

		filename = event.src_path
		if not filename:
			return

		if not os.path.exists(filename):
			return

		editor = Editor(self.notebook)
		self.notebook.add(editor, text=os.path.basename(filename))
		editor.open(filename, '..\\Case')

	def _onMonitorDeletedFile(self, event):
		if not self._casename:
			return

		filename = event.src_path
		tabs = [self.nametowidget(tab) for tab in self.notebook.tabs()]
		for editor in filter(lambda tab: isinstance(tab, Editor), tabs):
			if filename == editor.filename:
				self.notebook.forget(editor)
				break

	def _onMonitorModifiedFile(self, event):
		if not self._casename:
			return

		filename = event.src_path
		tabs = [self.nametowidget(tab) for tab in self.notebook.tabs()]
		for editor in filter(lambda tab: isinstance(tab, Editor), tabs):
			if filename == editor.filename:
				editor.reload()
				break

	def _onMonitorMovedFile(self, event):
		if not self._casename:
			return

		filename_src = event.src_path
		filename_dst = event.dest_path

		tabs = [self.nametowidget(tab) for tab in self.notebook.tabs()]
		for editor in filter(lambda tab: isinstance(tab, Editor), tabs):
			if filename_src == editor.filename:
				editor.rename(filename_dst)
				break

print( os.getcwd() )
root = Tk()
root.title('Case Tool')
root.geometry('800x600')

app = Application(root)
root.mainloop()