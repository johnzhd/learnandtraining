#include "stdafx.h"
#include "connect_ssl.h"

#ifdef USE_SSL

connect_ssl::connect_ssl(boost::asio::io_service& io_service, boost::asio::ssl::context& context)
	: socket_(io_service,context)
{
}


connect_ssl::~connect_ssl(void)
{
}

boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& connect_ssl::socket(){
	return socket_;
}

boost::asio::ip::tcp::endpoint connect_ssl::endpoint(){
	return socket_.lowest_layer().remote_endpoint();
}

connect_ssl::buff_array& connect_ssl::buff_temp(){
	return recv_buffer_;
}

void connect_ssl::clear(){
	boost::system::error_code ignored_ec;
	socket_.shutdown(ignored_ec);
	socket_.lowest_layer().close();

	memset( recv_buffer_.c_array(), 0, recv_buffer_.size() );
	//endpoint_;
}


#endif