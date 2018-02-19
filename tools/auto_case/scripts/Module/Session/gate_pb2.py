# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: gate.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='gate.proto',
  package='gate',
  syntax='proto3',
  serialized_pb=_b('\n\ngate.proto\x12\x04gate\"/\n\tlogin_req\x12\x10\n\x08username\x18\x01 \x01(\t\x12\x10\n\x08password\x18\x02 \x01(\t\"@\n\tlogin_ack\x12\x0e\n\x06result\x18\x01 \x01(\x05\x12\x0f\n\x07user_id\x18\x02 \x01(\x04\x12\x12\n\nchat_token\x18\x03 \x01(\t\"\x1d\n\nlogout_req\x12\x0f\n\x07user_id\x18\x01 \x01(\x04*a\n\x0bGATE_MSG_ID\x12\x12\n\x0eGATE_MSG_BEGIN\x10\x00\x12\x13\n\x0eGATE_LOGIN_REQ\x10\x81 \x12\x13\n\x0eGATE_LOGIN_ACK\x10\x82 \x12\x14\n\x0fGATE_LOGOUT_REQ\x10\x83 b\x06proto3')
)

_GATE_MSG_ID = _descriptor.EnumDescriptor(
  name='GATE_MSG_ID',
  full_name='gate.GATE_MSG_ID',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='GATE_MSG_BEGIN', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='GATE_LOGIN_REQ', index=1, number=4097,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='GATE_LOGIN_ACK', index=2, number=4098,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='GATE_LOGOUT_REQ', index=3, number=4099,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=166,
  serialized_end=263,
)
_sym_db.RegisterEnumDescriptor(_GATE_MSG_ID)

GATE_MSG_ID = enum_type_wrapper.EnumTypeWrapper(_GATE_MSG_ID)
GATE_MSG_BEGIN = 0
GATE_LOGIN_REQ = 4097
GATE_LOGIN_ACK = 4098
GATE_LOGOUT_REQ = 4099



_LOGIN_REQ = _descriptor.Descriptor(
  name='login_req',
  full_name='gate.login_req',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='username', full_name='gate.login_req.username', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='password', full_name='gate.login_req.password', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=20,
  serialized_end=67,
)


_LOGIN_ACK = _descriptor.Descriptor(
  name='login_ack',
  full_name='gate.login_ack',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='result', full_name='gate.login_ack.result', index=0,
      number=1, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='user_id', full_name='gate.login_ack.user_id', index=1,
      number=2, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='chat_token', full_name='gate.login_ack.chat_token', index=2,
      number=3, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=69,
  serialized_end=133,
)


_LOGOUT_REQ = _descriptor.Descriptor(
  name='logout_req',
  full_name='gate.logout_req',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='user_id', full_name='gate.logout_req.user_id', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=135,
  serialized_end=164,
)

DESCRIPTOR.message_types_by_name['login_req'] = _LOGIN_REQ
DESCRIPTOR.message_types_by_name['login_ack'] = _LOGIN_ACK
DESCRIPTOR.message_types_by_name['logout_req'] = _LOGOUT_REQ
DESCRIPTOR.enum_types_by_name['GATE_MSG_ID'] = _GATE_MSG_ID
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

login_req = _reflection.GeneratedProtocolMessageType('login_req', (_message.Message,), dict(
  DESCRIPTOR = _LOGIN_REQ,
  __module__ = 'gate_pb2'
  # @@protoc_insertion_point(class_scope:gate.login_req)
  ))
_sym_db.RegisterMessage(login_req)

login_ack = _reflection.GeneratedProtocolMessageType('login_ack', (_message.Message,), dict(
  DESCRIPTOR = _LOGIN_ACK,
  __module__ = 'gate_pb2'
  # @@protoc_insertion_point(class_scope:gate.login_ack)
  ))
_sym_db.RegisterMessage(login_ack)

logout_req = _reflection.GeneratedProtocolMessageType('logout_req', (_message.Message,), dict(
  DESCRIPTOR = _LOGOUT_REQ,
  __module__ = 'gate_pb2'
  # @@protoc_insertion_point(class_scope:gate.logout_req)
  ))
_sym_db.RegisterMessage(logout_req)


# @@protoc_insertion_point(module_scope)