#include "stdafx.h"
#include "connect_socket.h"


connect_socket::connect_socket(boost::asio::io_service& io_service)
	: socket_(io_service)
{
}


connect_socket::~connect_socket(void)
{
}

boost::asio::ip::tcp::socket& connect_socket::socket(){
	return socket_;
}

boost::asio::ip::tcp::endpoint connect_socket::endpoint(){
	return socket_.remote_endpoint();
}

connect_socket::buff_array& connect_socket::buff_temp(){
	return recv_buffer_;
}

void connect_socket::clear(){
	boost::system::error_code ignored_ec;
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	socket_.close();
	memset( recv_buffer_.c_array(), 0, recv_buffer_.size() );
	//endpoint_;
}