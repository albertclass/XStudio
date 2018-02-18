from functools import *

import tkinter as tk

import tkinter.ttk as ttk
import tkinter.font

class TreeviewEx(ttk.Treeview):
    def __init__(self, master=None, **kw):
        ttk.Treeview.__init__(self, master, **kw)

        self._curfocus = None
        self._curfocus_col = None
        self._inplace = { None : {} }
        '''
        self._inplace = {
            None = {
                col = (widget, var, show),
            }

            item = {
                col = (widget, var, show),
            }
        }
        '''
        self.bind('<ButtonRelease-1>', self.__button_check_focus)

        #Wheel events
        self.bind('<MouseWheel>', lambda e: self.after_idle(self.__updateWnds))
        self.bind('<space>', self.__key_check_focus)
        self.bind('<Return>', self.__key_check_focus)
        self.bind('<Home>', partial(self.__on_key_press, 'Home'))
        self.bind('<End>', partial(self.__on_key_press, 'End'))
        self.bind('<Configure>',
            lambda e: self.after_idle(self.__updateWnds))

    def __on_key_press(self, key, event):
        if key == 'Home':
            self.selection_set("")
            self.focus(self.get_children()[0])
        if key == 'End':
            self.selection_set("")
            self.focus(self.get_children()[-1])

    def delete(self, *items):
        self.after_idle(self.__updateWnds)
        ttk.Treeview.delete(self, *items)

    def yview(self, *args):
        """Update inplace widgets position when doing vertical scroll"""
        self.after_idle(self.__updateWnds)
        ttk.Treeview.yview(self, *args)

    def yview_scroll(self, number, what):
        self.after_idle(self.__updateWnds)
        ttk.Treeview.yview_scroll(self, number, what)

    def yview_moveto(self, fraction):
        self.after_idle(self.__updateWnds)
        ttk.Treeview.yview_moveto(self, fraction)

    def xview(self, *args):
        """Update inplace widgets position when doing horizontal scroll"""
        self.after_idle(self.__updateWnds)
        ttk.Treeview.xview(self, *args)

    def xview_scroll(self, number, what):
        self.after_idle(self.__updateWnds)
        ttk.Treeview.xview_scroll(self, number, what)

    def xview_moveto(self, fraction):
        self.after_idle(self.__updateWnds)
        ttk.Treeview.xview_moveto(self, fraction)

    def __button_check_focus(self, event):
        """Checks if the focus has changed"""
        newfocus = self.focus()
        ele = self.identify_element(event.x, event.y)
        if ele == 'Treeitem.indicator':
            return

        col = self.identify_column(event.x)
        if newfocus and self._curfocus == newfocus:
            self.__focus(newfocus,event, col)
            self.__updateWnds()
        else:
            self.__close_inplace_widgets()
            self._curfocus = newfocus

    def __key_check_focus(self, event):
        newfocus = self.focus()

        for sel in self.selection():
            newfocus = sel

        if newfocus:
            self.__focus(newfocus,event)
            self.__updateWnds()

        return 'break'
    def __identify_colunm(self, col):
        if col[0] != '#':
            return col
        
        display = self.cget('displaycolumns')
        if display == ('#all',):
            display = ('#0',) + self.cget('columns')
        else:
            display = ('#0',) + display

        idx = int(col[1:])
        return display[idx]

    def __focus(self, item, event, col='#0'):
        """Called when focus item has changed"""
        conf = self._inplace[item] if item in self._inplace else self._inplace[None]
        self._curfocus = item
        self._curfocus_col = col = self.__identify_colunm(col)

        self.__event_info =(col,item)
        self.event_generate('<<TreeviewInplaceEdit>>')

        if col in conf:
            conf[col][0].focus_set()


    def __updateWnds(self, event=None):
        if not self._curfocus:
            return

        item = self._curfocus
        if not self.exists(item):
            return

        col = self._curfocus_col
        if col not in self.cget('columns') + ('#0',):
            return

        confs = self._inplace[item] if item in self._inplace else self._inplace[None]

        for col, conf in confs.items():
            wnd = conf[0]
            var = conf[1]
            bbox = self.bbox(item, column=col)
            wnd.place(in_=self, x=bbox[0], y=bbox[1],width=bbox[2], height=bbox[3])
            var.set( self.__get_value(col, item) )


    def __close_inplace_widgets(self, item=None):
        item = item or self._curfocus
        conf = self._inplace[item] if item in self._inplace else self._inplace[None]

        for col, cfg in conf.items():
            cfg[0].place_forget()

    def __clear_inplace_widgets(self):
        """Remove all inplace edit widgets."""
        for item, conf in self._inplace.items():
            for col, cfg in conf.items():
                cfg[0].place_forget()
        
        self.focus_set()

    def get_event_info(self):
        return self.__event_info;

    def __get_inplace_configura(self, col, item):
        if item not in self._inplace:
            item = None

        if col not in self._inplace[item]:
            return None
        
        return self._inplace[item][col]
        

    def __get_value(self, col, item):
        item = item or self._curfocus
        if col == '#0':
            return self.item(item, 'text')
        else:
            return self.set(item, col)

    def __set_value(self, col, item, value):
        item = item or self._curfocus

        if col == '#0':
            self.item(item, text=value)
        else:
            self.set(item, col, value)
        self.__event_info =(col,item)
        self.event_generate('<<TreeviewCellEdited>>')

    def __update_value(self, col, item):
        item = item or self._curfocus
        
        if not self.exists(item):
            return

        configura = self.__get_inplace_configura(col, item)
        if configura:
            oldvalue = self.__get_value(col, item)
            newvalue = configura[1].get()
            if oldvalue != newvalue:
                self.__set_value(col, item, newvalue)

    def inplace_forget(self, col, item):
        configura = self.__get_inplace_configura(col, item)
        if configura:
            del configura

    def inplace_entry(self, col, item=None):
        widget = ttk.Entry(self, textvariable=tk.StringVar())
        return self.inplace_custom(col, item, widget)


    def inplace_checkbutton(self, col, item=None):
        widget = ttk.Checkbutton(self, variable=tk.StringVar(), onvalue='True', offvalue='False')
        return self.inplace_custom(col, item, widget, varbind='variable')


    def inplace_combobox(self, col, item=None, **kw):
        widget = ComboboxEx(self, textvariable=tk.StringVar(), **kw)
        return self.inplace_custom(col, item, widget)


    def inplace_spinbox(self, col, item=None, **kw):
        widget = tk.Spinbox(self, textvariable=tk.StringVar(), **kw)
        return self.inplace_custom(col, item, widget)


    def inplace_custom(self, col, item, widget, varbind='textvariable'):
        svar = tk.Variable(name=widget.cget(varbind))

        if item not in self._inplace:
            self._inplace[item] = {}

        self._inplace[item][col] = (widget, svar, True)

        widget.bind('<Unmap>', lambda e: self.__update_value(col, item))
        widget.bind('<FocusOut>', lambda e: self.__update_value(col, item))
        widget.bind('<Escape>', lambda e: self.__clear_inplace_widgets())
        widget.bind('<Key-Tab>',
            lambda e: w.tk_focusNext().focus_set())
        widget.bind('<Shift-Key-Tab>',
            lambda e: w.tk_focusPrev().focus_set())

