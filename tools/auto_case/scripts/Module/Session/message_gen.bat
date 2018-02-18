python message_gen.py ..\proto\client_msg.h ..\proto\game_msg.h ..\proto\gate_msg.h ..\proto\battle_msg.h ..\proto\friend_msg.h ..\proto\cross_msg.h  
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\cross.proto 
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\client.proto
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\game.proto
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\gate.proto
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\battle.proto
..\proto\protoc --proto_path=..\proto\ --python_out=.\ ..\proto\friend.proto
pause