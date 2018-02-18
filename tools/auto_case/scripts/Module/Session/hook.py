from . import gate_pb2 as gate
from . import chat_pb2 as chat

from . import func
hook_tbl = {
	gate.GATE_LOGIN_ACK : func.onGateLoginAck,
	chat.CHAT_USERAUTH_ACK : func.onUserAuthAck,
	chat.CHAT_CHANNEL_ENTER_NTF : func.onChannelEnterNtf,

	chat.CHAT_USERINFO_ACK : func.onUserInfoAck,
	chat.CHAT_CHANNEL_ENTER_ACK : func.onChannelEnterAck,
	chat.CHAT_CHANNEL_LEAVE_ACK : func.onChannelLeaveAck,
	chat.CHAT_CHANNEL_USER_ACK : func.onChannelUserAck,

	chat.CHAT_USER_CHAT_NTF : func.onUserChatNtf,
	chat.CHAT_CHANNEL_CHAT_NTF : func.onChannelChatNtf,

	chat.CHAT_CHAT_ERR : func.onChatError,
	chat.CHAT_SYSTEM_CHAT_NTF : func.onSystemChatNtf,
}
