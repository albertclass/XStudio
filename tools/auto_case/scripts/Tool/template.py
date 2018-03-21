from parse import *
import restrict

function = type(lambda : None)

template = {
	'menu' : '新建测试用例',
	'text' : '',
	'name' : '*',
	'tags' : 'root', # 节点名
	'mode' : 'repeat', # 模式，repeat - 可重复， once - 只出现一次
	'type' : dict, # 节点类型 dict - 字典， tuple - 元组， list - 列表， int - 数值， str - 串值
	'edit' : ('#0', 'text', {'update' : '2'}),
	'children' : [
		{
			'menu' : '设置引用',
			'text' : '引用自',
			'name' : 'inherit',
			'tags' : 'inherit', 
			'mode' : 'once',
			'type' : dict,
			'show' : show_inherit,
			'sort' : 0,
			'prase' : prase_by_text,
			'edit' : ('1', 'text'),
			'children' : []
		},

		{
			'menu' : '创建节点',
			'text' : '节点',
			'name' : 'libs',
			'tags' : 'libs',
			'mode' : 'once',
			'type' : dict,
			'sort' : 1,
			'children' : []
		},

		{
			'menu' : '新建测试',
			'text' : '',
			'name' : '*',
			'tags' : 'test', 
			'mode' : 'repeat', 
			'type' : dict,
			'edit' : ('#0', 'text', {'update' : '2'}),
			'children' : [ 
				{
					'menu' : '创建预置列表',
					'text' : '预设列表',
					'name' : 'prepare',
					'tags' : 'prepare', 
					'mode' : 'once', 
					'type' : [function, list, tuple],
					'show' : show_prepare,
					'sort' : 0,
					'children' : [
						{
							'menu' : '添加预置引用',
							'text' : '预设项',
							'tags' : 'refrence', 
							'mode' : 'repeat', 
							'type' : str,
							'when' : [list, tuple],
							'show' : show_text,
							'edit' : ('1', 'text'),
						},

						{
							'menu' : '添加预置函数',
							'text' : '预设项',
							'tags' : 'function',
							'mode' : 'repeat',
							'type' : function,
							'when' : [list, tuple],
							'show' : show_func,
							'edit' : ('1', 'function'),
						},
					]
				},

				{
					'menu' : '新建约束',
					'text' : '约束',
					'name' : 'restrict',
					'tags' : 'restrict', 
					'mode' : 'once', 
					'type' : dict,
					'sort' : 1,
					'children' : [
						{
							'menu' : '新建业务逻辑',
							'text' : '业务逻辑',
							'name' : 'node',
							'tags' : 'node',
							'mode' : 'once',
							'type' : dict,
							'sort' : 0,
							'build' : build_by_node,
							'prase' : prase_by_node,
							'children' : [
								{
									'menu' : '添加消息约束',
									'text' : '消息约束',
									'name' : 'class',
									'tags' : 'message',
									'mode' : 'repeat',
									'type' : restrict.message,
									'show' : show_inspect,
									'build' : build_by_bind,
									'prase' : prase_by_bind,
									'children' : [
										{
											'text' : '消息号',
											'name' : 'msgid',
											'tags' : 'msgid',
											'mode' : 'bind',
											'type' : int,
											'show' : show_msgid,
											'make' : True,
											'delete' : False,
											'edit' : ('1', 'message'),
										},

										{
											'menu' : '添加数据检查',
											'text' : '数据检查',
											'name' : 'cdata',
											'tags' : 'cdata', 
											'mode' : 'bind', 
											'type' : function,
											'show' : show_func,
											'edit' : ('1', 'function'),
										},

										{
											'menu' : '添加逻辑检查',
											'text' : '检查逻辑',
											'name' : 'logic',
											'tags' : 'logic', 
											'mode' : 'bind', 
											'type' : function,
											'show' : show_func,
											'edit' : ('1', 'function'),
										},
									]
								},

								{
									'menu' : '添加顺序约束',
									'text' : '顺序约束',
									'name' : 'instance',
									'tags' : 'node',
									'mode' : 'repeat',
									'recursion' : True,
									'type' : restrict.sequence_node,
									'show' : show_node,
									'prase' : lambda tree, node, conf, writer: writer.write('restrict.sequence_node()'),
								},

								{
									'menu' : '添加收集约束',
									'text' : '收集约束',
									'name' : 'instance',
									'tags' : 'node',
									'mode' : 'repeat',
									'recursion' : True,
									'type' : restrict.collect_node,
									'show' : show_node,
									'prase' : lambda tree, node, conf, writer: writer.write('restrict.collect_node()'),
								},

								{
									'menu' : '添加选择约束',
									'text' : '选择约束',
									'name' : 'instance',
									'tags' : 'node',
									'mode' : 'repeat',
									'recursion' : True,
									'type' : restrict.switch_node,
									'show' : show_node,
									'prase' : lambda tree, node, conf, writer: writer.write('restrict.select_node()'),
								},
							]
						},

						{
							'menu' : '新建超时约束',
							'text' : '超时约束',
							'name' : 'timeout',
							'tags' : 'timeout',
							'mode' : 'once',
							'type' : int,
							'make' : True,
							'sort' : 1,
							'show' : show_text,
							'edit' : ('1', 'spin'),
							'delete' : False,
						},

						{
							'menu' : '新建忽略列表',
							'text' : '忽略列表',
							'name' : 'ignores',
							'tags' : 'ignores',
							'mode' : 'once', 
							'type' : [list, tuple],
							'make' : True,
							'sort' : 2,
							'show' : show_list,
							'children' : [
								{
									'menu' : '添加忽略消息',
									'text' : '忽略消息',
									'tags' : 'msgid', 
									'mode' : 'repeat', 
									'type' : int,
									'when' : [list, tuple],
									'show' : show_msgid,
									'edit' : ('1', 'message'),
								}
							]
						},
					]
				},

				{
					'menu' : '新建检查点',
					'text' : '检查点',
					'name' : 'check',
					'tags' : 'check', 
					'mode' : 'once', 
					'type' : [function, dict],
					'edit' : ('1', 'function'),
					'sort' : 2,
					'children' : [
						{
							'menu' : '新建验证列表',
							'text' : '验证列表',
							'name' : 'data_verify',
							'tags' : 'verify',
							'mode' : 'once',
							'type' : [list, tuple],
							'when' : dict,
							'children' : [
								{
									'menu' : '新建验证项',
									'text' : '验证项',
									'tags' : 'data',
									'mode' : 'repeat',
									'type' : [list, tuple],
									'build' : build_by_list,
									'prase' : prase_by_list_singleline,
									'children' : [
										{
											'text' : '数据',
											'tags' : 'user',
											'mode' : 'once',
											'type' : str,
											'when' : [list, tuple],
											'make' : True,
											'show' : show_text,
											'edit' : ('1', 'text'),
											'delete' : False,
										},

										{
											'text' : '期望',
											'tags' : 'except',
											'mode' : 'once',
											'type' : str,
											'when' : [list, tuple],
											'make' : True,
											'show' : show_text,
											'edit' : ('1', 'text'),
											'delete' : False,
										},

										{
											'text' : '比较函数',
											'tags' : 'compair',
											'mode' : 'once',
											'type' : [function, type(str.__init__)],
											'when' : [list, tuple],
											'make' : True,
											'show' : show_func,
											'edit' : ('1', 'function'),
											'delete' : False,
										},
									]
								},

								{
									'menu' : '新建验证项',
									'text' : '验证项',
									'tags' : 'function',
									'mode' : 'repeat',
									'type' : function,
									'show' : show_func,
									'edit' : ('1', 'function'),
								}
							]
						},

						{
							'menu' : '新建触发 - 成功',
							'text' : '成功触发',
							'name' : 'throw',
							'tags' : 'throw',
							'mode' : 'once',
							'type' : str,
							'make' : True,
							'when' : dict,
							'show' : show_text,
							'edit' : ('1', 'text'),
							'delete' : False,
						}
					]
				},

				{
					'menu' : '新建触发 - 失败',
					'text' : '失败触发',
					'name' : 'throw',
					'tags' : 'throw',
					'mode' : 'once',
					'type' : str,
					'sort' : 3,
					'make' : True,
					'show' : show_text,
					'edit' : ('1', 'text'),
					'delete' : False,
				}
			]
		},
	]
}

def get_template_children(path):
	children = [template] # make list can find child
	for field in path:
		for child in children:
			# find tags in children
			if field != child['tags']:
				# not this
				continue
			
			if 'recursion' in child and child['recursion']:
				# recursion node
				break

			# found field
			if 'children' in child:
				children = child['children']
				break
		else:
			# not found
			break
	else:
		return children

	return None

def get_template_node(path):
	children = [template]
	node = None
	for field in path:
		for child in children:
			# find tags in children
			if field != child['tags']:
				# not this
				continue
			
			if 'recursion' in child and child['recursion']:
				# recursion node
				break
			
			node = child
			# found field
			if 'children' in child:
				children = child['children']
			else:
				children = None
			
			break
		else:
			break
	else:
		return node

	return None
