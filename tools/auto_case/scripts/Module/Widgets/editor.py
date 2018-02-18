import os
import sys
import tkinter as tk
import tkinter.ttk as ttk

from tkinter.filedialog import asksaveasfile
from tkinter.messagebox import askyesno

from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.styles import get_style_by_name


class Editor(tk.Frame):

    def __init__(self, master=None, **kw):
        super().__init__(master, **kw)
        self.filename = None
        self.rootpath = None
        self.filestat = None
        self.modified = False
        self._pressed_ctrl = []

        self.lexer = get_lexer_by_name('python3')
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)

        self.editor = tk.Text(self, bg='#FFF', undo=True, wrap=tk.NONE)
        self.editor.grid(row=0, column=0, sticky=tk.NSEW)

        self.scroll_y = tk.Scrollbar(self, orient=tk.VERTICAL, command=self.editor.yview)
        self.scroll_y.grid(row=0, column=1, sticky=tk.NS)

        self.scroll_x = tk.Scrollbar(self, orient=tk.HORIZONTAL, command=self.editor.xview)
        self.scroll_x.grid(row=1, column=0, sticky=tk.EW)

        self.editor['yscrollcommand'] = self.scroll_y.set
        self.editor['xscrollcommand'] = self.scroll_x.set

        # self.editor.bind('Ctrl+Z', self.editor.edit_undo)
        # self.editor.bind('Ctrl+U', self.editor.edit_redo)
        font = tk.font.Font(self.editor, self.editor.cget('font'))
        self.editor.config(tabs=(font.measure(' ' * 4),))
        self.editor.bind('<KeyPress>', self._event_key_pressed)
        self.editor.bind('<KeyRelease>', self._event_key_release)
        self.editor.bind('<Control-KeyPress-a>', self._event_selectall)
        self.editor.bind('<Control-KeyPress-x>', self._event_cut)
        self.editor.bind('<Control-KeyPress-c>', self._event_copy)
        self.editor.bind('<Control-KeyPress-v>', self._event_paste)
        self.editor.bind('<Control-KeyPress-z>', self._event_undo)
        self.editor.bind('<Control-KeyPress-y>', self._event_redo)

        self.editor.bind('<<Modified>>', self._modified_set)
        self.editor.bind('<FocusOut>', self._event_focus_leave)
        self._create_tags()

    def _modified_set(self, event=None):
        value = event.widget.edit_modified()

        filename = self.filename or 'untitled'
        if value and not self.modified:
            index = self.master.index(self)
            self.master.tab(index, text=os.path.basename(filename) + '*')
        elif self.modified and not value:
            index = self.master.index(self)
            self.master.tab(index, text=os.path.basename(filename))

        self.modified = value

    def _event_selectall(self, event=None):
        self.editor.tag_add(tk.SEL, '1.0', tk.END)
        self.editor.mark_set(tk.INSERT, '1.0')
        self.editor.see(tk.INSERT)

        return 'break'

    def _event_cut(self, event=None):
        if self.editor.tag_ranges(tk.SEL):
            self._event_copy()
            self.editor.delete(tk.SEL_FIRST, tk.SEL_LAST)
            self.recolorize()

        return 'break'

    def _event_copy(self, event=None):
        if self.editor.tag_ranges(tk.SEL):
            text = self.editor.get(tk.SEL_FIRST, tk.SEL_LAST)
            self.clipboard_clear()
            self.clipboard_append(text)

        return 'break'

    def _event_paste(self, event=None):
        text = self.editor.selection_get(selection='CLIPBOARD')
        self.paste(text)
        return 'break'

    def _event_undo(self, event=None):
        self.editor.edit_undo()
        self.recolorize()
        return 'break'

    def _event_redo(self, event=None):
        self.editor.edit_redo()
        self.recolorize()
        return 'break'

    def _event_focus_leave(self, event=None):
        self._pressed_ctrl = []

    def _event_key_pressed(self, event=None):
        if event.char:
            return

        if event.keysym in self._pressed_ctrl:
            return

        self._pressed_ctrl.append(event.keysym)
        # print( 'pressed', self._pressed_ctrl )

    def _event_key_release(self, event=None):
        # print('--------------------------')
        # pos = self.editor.index(tk.INSERT)
        # print(pos)

        # pos = self.editor.index('insert linestart')
        # print(pos)

        # pos = self.editor.index('insert lineend')
        # print(pos)

        if event.char:
            if len(self._pressed_ctrl) == 0:
                self.recolorize('insert linestart', 'insert lineend')

        if event.keysym in self._pressed_ctrl:
            self._pressed_ctrl.remove(event.keysym)
            # print( 'release', self._pressed_ctrl )

    def _create_tags(self):
        bold_font = tk.font.Font(self.editor, self.editor.cget('font'))
        bold_font.configure(weight=tk.font.BOLD)

        italic_font = tk.font.Font(self.editor, self.editor.cget('font'))
        italic_font.configure(slant=tk.font.ITALIC)

        bold_italic_font = tk.font.Font(self.editor, self.editor.cget('font'))
        bold_italic_font.configure(weight=tk.font.BOLD, slant=tk.font.ITALIC)

        style = get_style_by_name('default')
        for ttype, ndef in style:

            if ndef['bold'] and ndef['italic']:
                tag_font = bold_italic_font
            elif ndef['bold']:
                tag_font = bold_font
            elif ndef['italic']:
                tag_font = italic_font
            else:
                tag_font = None

            if ndef['color']:
                foreground = '#%s' % ndef['color']
            else:
                foreground = None

            self.editor.tag_configure(str(ttype), foreground=foreground, font=tag_font)

    def _update_filestat(self, filestat=None):
        with open(self.filename) as file:
            if filestat:
                return os.fstat(file.fileno()) == filestat
            else:
                self.filestat = os.fstat(file.fileno())
                return True

        return False

    def paste(self, text):
        if text:
            self.editor.insert(tk.INSERT, text)
            self.editor.tag_remove(tk.SEL, '1.0', tk.END)
            self.editor.see(tk.INSERT)
            self.recolorize()

    def clear(self):
        self.editor.delete('0.0', tk.END)

    def replace(self, text, start='1.0', end=tk.END):
        self.editor.replace(start, end, text)
        self.recolorize()
        
    def recolorize(self, start='1.0', end='end-1c'):
        code = self.editor.get(start, end)
        tokensource = self.lexer.get_tokens(code)

        start = self.editor.index(start)
        end = self.editor.index(end)

        start_line, start_index = (int(x) for x in start.split('.'))
        end_line, end_index = start_line, start_index

        for token, value in tokensource:
            if '\n' in value:
                end_line += value.count('\n')
                end_index = len(value.split('\n', 1)[-1])
            else:
                end_index += len(value)

            if value not in (' ', '\n'):
                index1 = '%s.%s' % (start_line, start_index)
                index2 = '%s.%s' % (end_line, end_index)

                for tag_name in self.editor.tag_names(index1):
                    self.editor.tag_remove(tag_name, index1, index2)

                self.editor.tag_add(str(token), index1, index2)

            start_line, start_index = end_line, end_index

    def open(self, filename, rootpath):
        self.filename = os.path.abspath(filename)
        self.rootpath = os.path.abspath(rootpath)
        if not os.path.exists(self.filename):
            self.editor.edit_modified(True)
            return False

        success = False
        with open(filename, 'rt', encoding='utf-8') as file:
            text = file.read()
            self.clear()
            self.paste(text)
            self.editor.edit_reset()
            self.editor.edit_separator()
            self.editor.edit_modified(False)
            self.event_generate('<<open>>')
            success = True

        self._update_filestat()
        return success

    def save(self, filename=None, rootpath=None):
        if not filename:
            filename = self.filename

        if not filename:
            filename = asksaveasfile(title='Select save to ...')

        if not filename:
            return False

        if rootpath:
            self.rootpath = rootpath

        success = False
        with open(filename, 'wt', encoding='utf-8') as file:
            text = self.editor.get('1.0', 'end')
            file.write(text)
            self.editor.edit_modified(False)
            success = True
        
        if success:
            self.event_generate('<<save>>')
            self._update_filestat()

        return success

    def reload(self):
        if self.editor.edit_modified():
            answer = askyesno('Quession', 'file has been modified. reload it?')
            if answer == False:
                return

        if self._update_filestat(self.filestat):
            return True

        with open(self.filename, 'rt', encoding='utf-8') as file:
            text = file.read()
            self.clear()
            self.paste(text)
            self.editor.edit_separator()
            self.editor.edit_modified(False)
            self.event_generate('<<reload>>')
            return True

        return False

    def rename(self, filename):
        if not filename:
            return

        self.filename = filename
        index = self.master.index(self)

        if self.modified:
            self.master.tab(index, text=os.path.basename(filename) + '*')
        else:
            self.master.tab(index, text=os.path.basename(filename))

        self.event_generate('<<rename>>')

    def close(self):
        if self.is_modified():
            answer = askyesno('Quession', 'file has been modified. save it?')
            if answer == False:
                return
            else:
                self.save()

        self.master.forget(self)

    def is_modified(self):
        return self.editor.edit_modified()
