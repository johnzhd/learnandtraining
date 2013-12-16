#pragma once

#ifdef USE_SSL

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>


class connect_ssl
	: public boost::enable_shared_from_this<connect_ssl>,
	private boost::noncopyable
{
public:
	explicit connect_ssl(boost::asio::io_service& io_service, boost::asio::ssl::context& context);
	~connect_ssl(void);
public:
	typedef boost::array<unsigned char, 8192> buff_array;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket();
	boost::asio::ip::tcp::endpoint endpoint();
	buff_array& buff_temp();

	void clear();
private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

	buff_array recv_buffer_;
};


/// ssl ///
typedef boost::shared_ptr<connect_ssl> connection_ssl;

#endif