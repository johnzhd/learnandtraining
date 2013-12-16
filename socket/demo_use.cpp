#include "stdafx.h"
#include "demo_use.h"

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>


demo_use::demo_use(void)
{
}


demo_use::~demo_use(void)
{
}

void demo_use::run_server(){
	my_server.run();
}


void demo_use::handler_accept( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e ){
	
	if ( e ) {
		//OUTPUT_DEBUG( e.message() );
		connect_ptr_->clear();
		return ;
	}
	printf_s( "[accept] handler: %s:%d\n", connect_ptr_->endpoint().address().to_string().c_str(), connect_ptr_->endpoint().port() ); 
	//save connect_ptr_
	//boost::asio::ip::tcp::endpoint accept_endpoint_;
	//accept_endpoint_ = connect_ptr_->socket().remote_endpoint();
	//std::string str_ = accept_endpoint_.address().to_string();
	//unsigned short port_ = accept_endpoint_.port();

	my_server.recv(connect_ptr_, 10, &demo_use::handler_recv, shared_from_this(), reinterpret_cast<LPVOID>(NULL) );

	my_server.start_accept( m_listenor_ptr_, &demo_use::handler_accept, shared_from_this(), reinterpret_cast<LPVOID>(NULL) );
}

void demo_use::handler_recv( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e, size_t bytes_transferred_ ){
	
	if ( e ) {
		//OUTPUT_DEBUG( e.message() );
		connect_ptr_->clear();
		return ;
	}
	printf_s( "[recv] handler: %s:%d (%d)\n", connect_ptr_->endpoint().address().to_string().c_str(), connect_ptr_->endpoint().port(), bytes_transferred_ ); 
	std::string str( reinterpret_cast<char*>( connect_ptr_->buff_temp().c_array()), bytes_transferred_);

	printf_s( "%s\n", str.c_str() );

	connect_ptr_->clear();
	//my_server.recv(connect_ptr_, 0, &demo_use::handler_recv, shared_from_this(), reinterpret_cast<LPVOID>( NULL  ));
}



void demo_use::handler_connect( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e ){
	
	if ( e ) {
		//OUTPUT_DEBUG( e.message() );
		std::string str = e.message();
		int d = e.value();
		connect_ptr_->clear();
		return ;
	}
	printf_s( "[conn] handler: %s:%d\n", connect_ptr_->endpoint().address().to_string().c_str(), connect_ptr_->endpoint().port() );


}

void demo_use::handler_send( LPVOID lparam, connection_ptr connect_ptr_, const boost::system::error_code& e ){
	if ( e ) {
		//OUTPUT_DEBUG( e.message() );
		connect_ptr_->clear();
		return ;
	}
	printf_s( "[send] handler: %s:%d\n", connect_ptr_->endpoint().address().to_string().c_str(), connect_ptr_->endpoint().port() ); 
	
}


void demo_use::test_1(){

	HANDLE h;
	h = CreateEvent( NULL, FALSE, FALSE, NULL );

	sync_single ds;
	//boost::function<void ()> g = boost::ref(ds);
	

	connection_ptr ptr_ = my_server.create_connect( std::string("www.baidu32.com"), std::string("80")
		, &demo_use::handler_connect_s
		, shared_from_this(), ds.call() );

	ds.Wait();

	boost::array<unsigned char, 64> array_;
	std::string str_( "GET / HTTP/1.1\r\nAccept: *.*\r\nHost: weibo.com\r\n\r\n\r\n" );
	memcpy( array_.c_array(), str_.c_str(), str_.length()   );
	my_server.send( ptr_, array_.c_array(), str_.length(), &demo_use::handler_send_s, shared_from_this(), ds.call() );

	ds.Wait();

	my_server.recv( ptr_, 10, &demo_use::handler_recv_s, shared_from_this(), ds.call() );

	ds.Wait();


	int a=1;
	a++;
	++a;
}

///////////////////////////////////////////////////////////////////////////////////////

DWORD thread_server_test( boost::shared_ptr<demo_use> ptr_ ){
	Sleep(20);
	ptr_->test_1();
	return 0;
}

void run(){
	/*
	HANDLE hEvent = CreateEvent( NULL,FALSE, FALSE, NULL );
	DWORD n1 = GetTickCount();

	boost::thread tt( [&](){if(hEvent!=NULL){Sleep(500);SetEvent(hEvent);}} );

	WaitForSingleObject(hEvent, INFINITE );
	DWORD n2 = GetTickCount();

	printf_s( "Test tick count = %d \n", n2 -n1 );

	CloseHandle(hEvent);
	return ;
	*/
	




	boost::shared_ptr<demo_use> p_use( new demo_use() );

	boost::thread t(boost::bind( thread_server_test,p_use));
	//t.join();
	try {
		p_use->run_server();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}


}