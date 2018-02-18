'''
ids.py 生成器
通过服务器的.h 文件生成python的对应消息映射脚本
'''
# 预定义消息类型
message_type = {
    'eMSG_TYPE_EVENT'     : 0,  # 对应通用事件，如网络，服务注册等
    'eMSG_TYPE_CLIENT'    : 1,
    'eMSG_TYPE_LOGIN'     : 2,
    'eMSG_TYPE_GATE'      : 3,
    'eMSG_TYPE_GAME'      : 4,
    'eMSG_TYPE_CENTER'    : 5,
    'eMSG_TYPE_FRIEND'    : 6,
    'eMSG_TYPE_BATTLE'    : 7,
    'eMSG_TYPE_CROSS'     : 8,
    'eMSG_TYPE_COUNT'     : 9,  # 最大数量
}

if __name__ == '__main__':
    import os
    import sys
    import re

    messages = []

    # 打开文件进行分析
    for filename in sys.argv[1:]:
        print( "alalysis file %s" % filename )
        file = open( filename, 'r' )

        # 查看每一行
        for line in file:
            # 匹配枚举关键字，并析出枚举名
            m = re.match( r'\s*enum[ |\t]*(\w+)', line )
            if m:
                messages.append( (m.group(1), []) )

            # 匹配消息定义，并析出消息定义和消息号
            m = re.match( r'\s*(e[\w_]+)\s*=\s*SET_MSG_ID[(]\s*(e[\w_]+)\s*,\s*(\d+)', line )
            if m:
                messages[-1][1].append( m.groups() )

        file.close()

    # 打开ids.py
    outfile = open( 'ids.py', 'w' )

    # 循环枚举定义
    for message in messages:
        outfile.write( '# %s\n' % message[0] )
        print( 'generate message %s\n' % message[0] )

        # 等号对齐
        alignwidth = len( max( message[1], key=lambda e: len( e[0] ) )[0] )

        # 写消息定义
        for define in message[1]:
            msgname = define[0].ljust(alignwidth+1);
            print( '%s = %u' % ( msgname, (((message_type[define[1]] << 12) & 0xF000) | (int(define[2]) & 0x0FFF)) ) )
            outfile.write( '%s = %u\n' % ( msgname, (((message_type[define[1]] << 12) & 0xF000) | (int(define[2]) & 0x0FFF)) ) )

    outfile.write( '###############################################\n' )
    outfile.write( '# generate message dict\n' )
    outfile.write( '###############################################\n' )
    outfile.write( 'names = {}\n' )
    # 循环枚举定义
    for message in messages:
        outfile.write( '# %s\n' % message[0] )
        print( 'generate message name %s\n' % message[0] )

        # 写消息定义
        for define in message[1]:
            print( "names[%u] = %s" % ( (((message_type[define[1]] << 12) & 0xF000) | (int(define[2]) & 0x0FFF)), define[0] ) )
            outfile.write( "names[%u] = '%s'\n" % ( (((message_type[define[1]] << 12) & 0xF000) | (int(define[2]) & 0x0FFF)), define[0] ) )

    # 关闭文件
    outfile.close()
