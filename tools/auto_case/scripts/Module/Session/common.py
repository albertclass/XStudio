import sys
import types
import restrict

from config import option
from . import ids

'''
case = {
    'library' : {
    },

    # test describe
    # 测试名
    'testname' : {
        # 准备工作，通过函数向服务器发送预置数据
        'prepare' : ('library.prepare', lambda cli : sendsome( cli, a, b, c )),
        # 测试约束
        'restrict' : {
            # 约束节点
            'node' : {
                # 类名，可创建 inspect 或 node
                'instance' : restrict.inspect( ids.eCLIENT_GT_LOGIN_ACK ),
            }, 
            # 约束超时时间
            'timeout' : 5,
            # 约束期间内忽略的消息
            'ignores' : ( ids.eCLIENT_GS_TASK_INFO_NTF, ),

            # 约束通过后的检查
            'check' : {
                # 数据验证列表
                'data_verify' : (
                    # 验证列表里每一项包含( 验证字段, 验证数据, 验证逻辑 )
                    # 验证将自动搜索用户数据中的验证字段
                    #	 字段不存在则认为验证失败
                    # 	 字段存在则调用后面的比较函数
                    #	 __eq__ : 等于
                    #	 __ne__ : 不等
                    #	 __lt__ : 小于
                    #	 __gt__ : 大于
                    #	 __le__ : 小于等于
                    #	 __ge__ : 大于等于
                    ('basic.platform_id', 0, int.__eq__ ),
                    ('basic.user_name', '123456', str.__eq__ ),
                    # 也可以传入回调函数，调用将按 check_callback( cli ) 的形式调用, 返回 True 验证通过， False 验证失败
                    check_callback,
                    lambda cli : cli.user.basic.platform_id == 0,
                ),
                # 验证全部通过则抛出该关键字
                'throw' : 'enter_game',
            },
            # 检查也可以设置为回调，这种方式提供了更大的灵活性
            # 回调返回下一个测试的名字
            'check' : check_callback,
            # 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
            'throw' : 'entergame',
        }
    },
}
'''

def success( cli ):
    account.info( 'case %s-%d successful!' % ( cfg.option.import_name, cfg.option.index ) )
    case_report.record_some(1, Error='Success!!!')

def failed( cli ):
    account.info( 'case %s-%d failed!' % ( cfg.option.import_name, cfg.option.index ) )
    case_report.record_some(1, Error='Failed!!!')

'''
basic_case = {
    'login' : {
        # 准备工作
        'prepare' : None,
        # 测试约束
        'restrict' : {
            # 约束节点
            'node' : {
                # 类名
                'instance' : restrict.inspect( ids.eCLIENT_GT_LOGIN_ACK, lambda msgid, msg, cli : msg.error_code == 0 ),
            }, 
            # 约束超时时间
            'timeout' : 5,
            # 约束期间内忽略的消息
            'ignores' : ('*'),
        },

        # 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
        'throw' : 'entergame',
    },

    'entergame' : {
        # 准备工作
        'prepare' : None,
        # 测试约束
        'restrict' : {
            # 约束节点
            'node' : {
                # 类实例
                'instance' : restrict.sequence_node(),
                # 子节点 
                'children' : (
                    {
                        # 类实例
                        'instance' : restrict.inspect( ids.eCLIENT_GT_ENTER_GAME_ACK ),
                    },
                    {
                        # 类实例
                        'instance' : restrict.select_node(),
                        # 子节点 
                        'children' : (
                            {
                                # 类实例
                                'instance' : restrict.sequence_node(),
                                # 子节点 
                                'children' : (
                                    {
                                        # 类名
                                        'instance' : restrict.inspect( ids.eCLIENT_GS_SET_ROLE_NAME_ACK ),
                                    },
                                    {
                                        # 类名
                                        'instance' : restrict.inspect( ids.eCLIENT_GS_ENTER_GAME_NTF ),
                                    },)
                            },
                            {
                                # 类名
                                'instance' : restrict.inspect( ids.eCLIENT_GS_ENTER_GAME_NTF ),
                            },
                        ),
                    },
                ),
            },
            # 约束超时时间
            'timeout' : 5,
            # 约束期间内忽略的消息
            'ignores' : ('*'),
        },

        # 检查通过后抛出新的关键字约束，任何非约束内的关键字都将结束此次测试
        'throw' : 'ready',
    },

    'success' : {
        'check' : success
    },

    'failed' : {
        'check' : failed
    }
}
'''
