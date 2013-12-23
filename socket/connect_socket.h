#pragma once


#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>


class sync_single{
private:
	HANDLE hEvent;
public:
	sync_single():hEvent(NULL){hEvent = CreateEvent(NULL, FALSE,FALSE,NULL);Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));};
	~sync_single(){if ( hEvent ) CloseHandle( hEvent);Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));};
public:
	void operator() (){ if (hEvent) SetEvent(hEvent);};

	boost::function<void ()> call(){ return boost::ref(*this); };

	void Wait(){ if (hEvent) WaitForSingleObject(hEvent,INFINITE);};

};

class connect_socket
	: public boost::enable_shared_from_this<connect_socket>,
		private boost::noncopyable
{
public:
	explicit connect_socket(boost::asio::io_service& io_service);
	~connect_socket(void);
public:
	typedef boost::array<unsigned char, 8192> buff_array;
	boost::asio::ip::tcp::socket& socket();
	boost::asio::ip::tcp::endpoint endpoint();
	buff_array& buff_temp();

	void clear();
private:
	boost::asio::ip::tcp::socket socket_;

	buff_array recv_buffer_;
};


typedef boost::shared_ptr<connect_socket> connection_ptr;


