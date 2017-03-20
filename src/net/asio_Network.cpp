///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file asio_Network.cpp
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ����API��װ
///
///////////////////////////////////////////////////////////////
#include "Header.h"

namespace xgc
{
	namespace net
	{
		asio_Network::asio_Network( void )
			: service_( XGC_NEW asio::io_service() )
		{
		}

		asio_Network::~asio_Network( void )
		{
		}

		xgc_void asio_Network::run()
		{
			io_service::work w( *service_ );
			service_->run();
		}

		xgc_bool asio_Network::insert_workthread( xgc_int32 insert_workthread )
		{
			for( int i = 0; i < insert_workthread; ++i )
			{
				workthreads_.emplace_back( std::thread( std::bind( &asio_Network::run, this ) ) );
			}

			return true;
		}

		xgc_void asio_Network::exit()
		{
			service_->stop();

			for( size_t i = 0; i < workthreads_.size(); ++i )
			{
				workthreads_[i].join();
			}

			SAFE_DELETE( service_ );
		}

		///////////////////////////////////////////////////////
		asio_Network& getNetwork()
		{
			static asio_Network asio_network;
			return asio_network;
		}
	}
}
