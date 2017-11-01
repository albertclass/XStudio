#pragma once
#ifndef _NETWORK_H_
#define _NETWORK_H_

namespace xgc
{
	namespace net
	{
		///
		/// \brief ÍøÂç¿âÒýÇæ·â×°Àà
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:58
		///
		class asio_Network
		{
			friend asio_Network& getNetwork();

		private:
			asio_Network(void);
			~asio_Network(void);

			xgc_void run();	
		public:
			xgc_bool insert_workthread( xgc_int32 c );

			xgc_void exit();

			asio::io_service& Ref(){ return *service_; }
		private:
			asio::io_service *service_;
			std::vector< std::thread >	workthreads_;
		};

		///
		/// \brief get asio network
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		asio_Network& getNetwork();
	}
}
#endif // _NETWORK_H_