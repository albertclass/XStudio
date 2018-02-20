from .func import *
hook_tbl = {
	GATE.GATE_LOGIN_ACK : onGateLoginAck,
	CHAT.CHAT_USERAUTH_ACK : onUserAuthAck,
	CHAT.CHAT_CHANNEL_ENTER_NTF : onChannelEnterNtf,

	CHAT.CHAT_USERINFO_ACK : onUserInfoAck,
	CHAT.CHAT_CHANNEL_ENTER_ACK : onChannelEnterAck,
	CHAT.CHAT_CHANNEL_LEAVE_ACK : onChannelLeaveAck,
	CHAT.CHAT_CHANNEL_USER_ACK : onChannelUserAck,

	CHAT.CHAT_USER_CHAT_NTF : onUserChatNtf,
	CHAT.CHAT_CHANNEL_CHAT_NTF : onChannelChatNtf,

	CHAT.CHAT_CHAT_ERR : onChatError,
	CHAT.CHAT_SYSTEM_CHAT_NTF : onSystemChatNtf,
}
