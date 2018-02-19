from google.protobuf import descriptor_pb2 as _descriptor_pb2
from google.protobuf import message_factory as _factory
from google.protobuf import descriptor as _descriptor

from . import gate_pb2
from . import chat_pb2

__messages_fd = [
	_descriptor_pb2.FileDescriptorProto.FromString(gate_pb2.DESCRIPTOR.serialized_pb),
	_descriptor_pb2.FileDescriptorProto.FromString(chat_pb2.DESCRIPTOR.serialized_pb),
]

__messages_pb = _factory.GetMessages( __messages_fd )

__builtins__['gate'] = {k : v for k, v in gate_pb2.GATE_MSG_ID.items()}
__builtins__['chat'] = {k : v for k, v in chat_pb2.CHAT_MSG_ID.items()}

__builtins__['GATE'] = type('GateMsgID', (object,), {k : v for k, v in gate_pb2.GATE_MSG_ID.items()})
__builtins__['CHAT'] = type('ChatMsgID', (object,), {k : v for k, v in chat_pb2.CHAT_MSG_ID.items()})

gate_name = {v : k for k, v in gate_pb2.GATE_MSG_ID.items()}
chat_name = {v : k for k, v in chat_pb2.CHAT_MSG_ID.items()}

from .name import name_tbl
from .hook import hook_tbl

def make(msgid : int):
	try:
		return None if msgid is None else __messages_pb[name_tbl[msgid]]()
	except KeyError:
		WRN("# message parser does't found. %d" % (msgid))
		pass

	return None

def name(msgid):
	return gate_name.get(msgid) or chat_name.get(msgid)

__all__ = ['make', 'parse', 'name']

