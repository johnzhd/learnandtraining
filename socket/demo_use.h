#pragma once

#include "iocp_server.hpp"

#include <list>
#include <vector>



class demo_use
	:public boost::enable_shared_from_this<demo_use>,private boost::noncopyable
{
public:
	demo_use(void);
	~demo_use(void);
public:
	void run_server();
public:
	void test_1();

	void handler_accept( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e );

	void handler_connect( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e );

	void handler_recv( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e, size_t bytes_transferred_ );

	void handler_send( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e );

	
	void handler_connect_s( boost::function< void () > lp_func, connection_ptr connect_ptr_, const boost::system::error_code& e ){
		handler_connect( reinterpret_cast<LPVOID>(NULL), connect_ptr_, e );
		if ( lp_func )	lp_func();
	}

	void handler_recv_s( boost::function< void () > lp_func, connection_ptr connect_ptr_, const boost::system::error_code& e, size_t bytes_transferred_ ){
		handler_recv( reinterpret_cast<LPVOID>( NULL), connect_ptr_, e, bytes_transferred_ );
		if ( lp_func ) lp_func();
	}

	void handler_send_s( boost::function< void () > lp_func, connection_ptr connect_ptr_, const boost::system::error_code& e ){
		handler_send( reinterpret_cast<LPVOID>(NULL), connect_ptr_, e );
		if ( lp_func ) lp_func();
	};
private:
	iocp_server my_server;


private:
	listenor_ptr m_listenor_ptr_;



private:
};

EXTERN_C
{
	__declspec(dllexport) void run();
}