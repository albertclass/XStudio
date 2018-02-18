login = {
	'comment' : '登陆',
	'start' : {
		# 切换到此状态后立即执行
		'befor' : 'session.connect( option.host, option.port )',
		
		# 约束成功后执行
		'after' : {
			'check' : 'session.is_connected()',
			# 检查通过后抛出
			'throw' : 'certification',
		},

		# 默认抛出
		'throw' : 'failed',
	},

	'certification' : {
		# 发送消息
		'send' : {
			'msgid' : 'GATE.GATE_LOGIN_REQ',
			'field' : {
				'username' : 'session.username',
				'password' : 'session.password',
			}
		},
		# 测试约束
		'recv' : {
			# 约束节点
			# 类名
			'instance' : 'collect_node',
			'children' : (
				{
					'instance' : ( 'message', {
						'msgid' : 'GATE.GATE_LOGIN_ACK',
						'check' : call('msg.result == 0'),
						'count' : 1,
					} )
				},
			),

			# 约束超时时间
			'timeout' : 5,
			# 约束期间内忽略的消息
			'ignores' : '*',
		},

		# 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
		'throw' : 'failed',
	},

	'step_3' : {
		# 准备工作
		'send' : {
			'msgid' : 'ID_CLIENT_ENTER_REQ',
			'field' : {
				'token' : 'cli.user.token',
			},
		},

		# 测试约束
		'recv' : {
			# 类名
			'instance' : 'colloc_node',
			'children' : (
				{
					'msgid' : 'ids.CLIENT_LOGIN_RPN',
					'check' : 'msg.error_code == 0',
				},
			),
			# 约束超时时间
			'timeout' : 5,
			# 约束期间内忽略的消息
			'ignores' : '*',
		},

		'after' : {
			# 约束通过后的检查函数，该函数返回分支名
			'check' : 'login_check()',
		},

		# 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
		'throw' : 'failed',
	},
}

# CASE END

