#include "stdafx.h"
#include "system.h"


#include "iocp_server.hpp"

#include "thread_pool.h"

#include "policy_work.h"




void func_accept(listenor_ptr ptr, connection_ptr connection_, const boost::system::error_code& e)
{
	if ( e )
	{
		Noise_log("Server accept err: %d %s\n", e.value(), e.message());
	};
	connection_->clear();
	iocp_server_api::iocp_server_ptr net = iocp_server_api::get_server();
	net->start_accept( ptr, func_accept, ptr  );
};



std::string system_api::run( boost::function<void (bool)> func_init_finished, bool bReset )
{
	std::string err_string;
	size_t n = 0;
	do {
		iocp_server_api::iocp_server_ptr iocp_server_point = iocp_server_api::get_server();
		if ( iocp_server_point == nullptr )
		{
			High_log( "New net I/O pool failed.\n" );
			return system_log::err_ret(system_log::system_err(2));
		};
		thread_pool_api::thread_pool_ptr thread_pool_point = thread_pool_api::get_pool();
		if ( thread_pool_point == nullptr )
		{
			High_log( "New thread pool failed.\n" );
			return system_log::err_ret(system_log::system_err(3));
		};
		policy_api::policy_server_ptr policy_server_point = policy_api::get_server();
		if ( policy_server_point == nullptr )
		{
			High_log( "New policy pool failed.\n" );
			return system_log::err_ret(system_log::system_err(4));
		};

		High_log( "System start No.%d \n", ++n );

		try
		{
			// start policy server
			//...
			if ( false == policy_server_point->init() )
				return system_log::err_ret(system_log::system_err(5));

			// start task pool
			//...
			if ( thread_pool_point->start( 5 * 30 ) == false )
				return system_log::err_ret(system_log::system_err(6));

			// start control server
			//...
			auto lis = iocp_server_point->create_listen("0.0.0.0","18080");
			iocp_server_point->start_accept( lis,func_accept, lis  );

			func_init_finished(true);
			// start base io
			iocp_server_point->run();
		}
		catch (std::exception& e)
		{
			err_string = e.what();
			High_log( "System run exception: %s\n", err_string );
			func_init_finished(false);
		}
		// stop interaction server

		// stop task pool
		thread_pool_point->stop();

		// stop io
		iocp_server_point->stop();

		// clear policy
		policy_server_point->clear();

		High_log( "System stop No.%d \n", ++n );

	}while ( bReset );
	return err_string;
}

