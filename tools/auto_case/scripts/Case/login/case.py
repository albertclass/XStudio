login = {
	'comment' : '登陆',
	'start' : {
		# 切换到此状态后立即执行
		'befor' : (
			'cli.connect( "game", option.host, option.port )',
			'session.username = option.username',
			'session.password = option.password',
		),
		# 约束成功后执行
		'after' : (
			invoke('cli.connected("game") is True'),
		),

		# 检查通过后抛出
		'throw' : 'certification',
	},

	'certification' : {
		# 发送消息
		'send' : {
			'msgid' : 'GATE.GATE_LOGIN_REQ',
			'field' : {
				'username' : var('session.username'),
				'password' : var('session.password'),
			}
		},
		# 测试约束
		'recv' : {
			# 约束节点
			# 类名
			'class' : 'collect_node',
			'child' : (
				{
					'class' : 'message', 
					'msgid' : 'GATE.GATE_LOGIN_ACK',
					'check' : (
						invoke('msg.result == 0'),
						assign('session.user_id',   var('msg.user_id')),
						assign('session.token',     var('msg.chat_token')),
						assign('session.chat_host', var('msg.host')),
						assign('session.chat_port', var('msg.port')),
					),
					'count' : 1,
				},
			),

			# 约束超时时间
			'timeout' : 5,
			# 约束期间内忽略的消息
			'ignores' : (),
		},

		'after' : (
			invoke('session.token is not None'),
		),

		# 一个默认的转换状态
		'throw' : 'connect_chat',
	},

	'connect_chat' : {
		# 准备工作
		'befor' : {
			'cli.connect( "chat", session.chat_host, session.chat_port )',
		},

		'after' : (
			# 约束通过后的检查函数，该函数返回分支名
			invoke('cli.connected("chat") is True'),
		),

		'throw' : 'enter_chat',
	},

	'enter_chat' : {
		'send' : {
			'msgid' : 'CHAT.CHAT_USERAUTH_REQ',
			'field' : {
				'user_id' : var('session.user_id'),
				'token'   : var('session.token'),
			},
		},

		'recv' : {
			'class' : 'message',
			'msgid' : 'CHAT.CHAT_USERAUTH_ACK',
			'check' : (
				invoke('msg.result == 0'),
				invoke('session.user_id == msg.user_id'),
				assign('session.chat_id', var('msg.chat_id')),
				assign('session.chat_nick', var('msg.nick')),
				assign('session.chat_extra', var('msg.extra')),
			),

			'timeout' : 5,
			'ignores' : (),
		},

		'throw' : 'chat_login',
	}
}

# CASE END

