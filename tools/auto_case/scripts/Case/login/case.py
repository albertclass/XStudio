login = {
	'comment' : '登陆',
	'start' : {
		# 切换到此状态后立即执行
		'befor' : (
			'session.connect( option.host, option.port )',
			'session.username = option.username',
			'session.password = option.password',
		),
		# 约束成功后执行
		'after' : {
			'check' : getvar('session.is_connected()'),
		},

		# 检查通过后抛出
		'throw' : 'certification',
	},

	'certification' : {
		# 发送消息
		'send' : {
			'msgid' : 'GATE.GATE_LOGIN_REQ',
			'field' : {
				'username' : getvar('session.username'),
				'password' : getvar('session.password'),
			}
		},
		# 测试约束
		'recv' : {
			# 约束节点
			# 类名
			'instance' : ( 'collect_node', ),
			'children' : (
				{
					'instance' : ( 'message', {
						'msgid' : 'GATE.GATE_LOGIN_ACK',
						'check' : (
							verify('msg.result', 0),
							assign('session.user_id', 'msg.user_id'),
							assign('session.token', 'msg.chat_token'),
							assign('session.chat_host', 'msg.host'),
							assign('session.chat_port', 'msg.port'),
						),
						'count' : 1,
					} )
				},
			),

			# 约束超时时间
			'timeout' : 5,
			# 约束期间内忽略的消息
			'ignores' : (),
		},

		'after' : {
			'check' : getvar('session.token is not None'),
		},

		# 一个默认的转换状态
		'throw' : 'connect_chat',
	},

	'connect_chat' : {
		# 准备工作
		'befor' : {
			'session.connect( session.chat_host, session.chat_port )',
		},

		'after' : {
			# 约束通过后的检查函数，该函数返回分支名
			'check' : 'session.is_connected()',
		},

		'throw' : 'enter_chat',
	},

	'enter_chat' : {
		'send' : {
			'msgid' : 'CHAT.CHAT_USERAUTH_REQ',
			'field' : {
				'user_id' : getvar('session.user_id'),
				'token'   : getvar('session.token'),
			},
		},

		'recv' : {
			'instance' : ('message', {
				'msgid' : 'CHAT.CHAT_USERAUTH_ACK',
				'check' : (
					verify('msg.result', 0),
					verify('session.user_id', 'msg.user_id'),
					assign('session.chat_id', 'msg.chat_id'),
					assign('session.chat_nick', 'msg.nick'),
					assign('session.chat_extra', 'msg.extra'),
				),
			})
		},

		'throw' : 'chat_msg',
	}
}

# CASE END

