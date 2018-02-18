#pragma once

#ifndef _MESSAGE_DISPATCH_H
#define _MESSAGE_DISPATCH_H

#include <vector>
#include <map>

namespace xgc
{
	namespace common
	{
		template< class func_type, class id_type >
		struct functionRegister
		{
			void regist( id_type id, func_type func )
			{
				message[id] = func;
			}

			inline func_type get( id_type id )
			{
                xgc::unordered_map< id_type, func_type >::iterator i = message.find( id );

				if( i != message.end() )
					return i->second;
				else
					return xgc_nullptr;
			}

			inline func_type dismiss( id_type id )
			{
                xgc::unordered_map< id_type, func_type >::iterator i = message.find( id );

				if( i != message.end() )
				{
					func_type ret = i->second;
					message.erase( i );
					return ret;
				}
				else
					return xgc_nullptr;
			}

			xgc::unordered_map< id_type, func_type >	message;
		};

		template< class func_type >
		struct functionRegister< func_type, unsigned char >
		{
			void regist( unsigned char id, func_type func )
			{
				if( id >= message.size() )
				{
					message.resize( id + 1, xgc_nullptr );
				}

				message[id] = func;
			}

			inline func_type get( unsigned char id )
			{
				return id < message.size()?message[id]:xgc_nullptr;
			}

			inline func_type dismiss( unsigned char id )
			{
				func_type ret = id < message.size()?message[id]:xgc_nullptr;
				if( ret )
				{
					message[id] = xgc_nullptr;
				}
				return ret;
			}

			xgc::vector< func_type >	message;
		};

		#define IMPLEMENT_DISPATCHER_TABLE( N )\
			struct _DISPATCHER##N;\
			static _DISPATCHER##N __##N##_DISPATCHER;\

		#define BEGIN_DISPATCHER_TABLE( CLASS, N )\
		struct CLASS##::_DISPATCHER##N : public functionRegister< func, type >\
			{\
				typedef CLASS THISCLASS;\
				_DISPATCHER##N()\
				{\

		#define DECLARE_DISPATCH( ID, F )\
					regist( ID, &THISCLASS::##F );\

		#define END_DISPATCHER_TABLE( CLASS, N )\
				}\
			};\
			CLASS##::_DISPATCHER##N CLASS##::__##N##_DISPATCHER;

		#define DISPATCHER_GET( N, ID ) __##N##_DISPATCHER.get( ID )

		template< class handle, class functype = bool (handle::*)( const char*, size_t ), class idtype = unsigned char >
		class message_dispatcher
		{
		protected:
			typedef typename idtype		type;
			typedef typename functype	func;
		};
	}
}

#endif // _MESSAGE_DISPATCH_H