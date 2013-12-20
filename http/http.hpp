#pragma once

#include <algorithm>

#include <string>
#include <vector>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/locks.hpp>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "iocp_server.hpp"

#include "http_tools.hpp"

#define HTTP_ONE_PACKET_SIZE	(1460)

namespace http
{
	bool download_one( bool post, std::string url, std::string head,
		std::vector<unsigned char>& send_raw, std::vector<unsigned char>& recv_raw );
};

namespace http
{	
	// not thread safe !!!!!
	// NOT!!!
	//template<typename Connection_Ptr, typename byte_http>

	template<typename Connection_Ptr, typename byte_http = unsigned char>
	class downloader 
		: public boost::enable_shared_from_this<downloader<Connection_Ptr,byte_http>>,  /*<Connection_Ptr,byte_http>*/
		private boost::noncopyable
	{
	public:
		downloader():socket_ptr_(),
			iocp_server_ptr_(iocp_server_api::get_server()),
			address_(),port_(),
			buffer_send_(),buffer_recv_(),
			thread_wait_mutex(),thread_wait_condition()
		{};
		~downloader(){};
	private:
		Connection_Ptr socket_ptr_;
		iocp_server_api::iocp_server_ptr iocp_server_ptr_;
		std::string address_;
		std::string port_;
		std::vector<byte_http> buffer_send_;
		std::vector<byte_http> buffer_recv_;

		boost::function< void (std::vector<byte_http>&) > end_function_;
		

		boost::mutex thread_wait_mutex; // for action
		boost::condition thread_wait_condition; // wait action signel


	public:
		// if raw_recv != NULL
		// download blocked until finished
		// if want to recv data through lp_func & Don't blocked.
		// set raw_recv = NULL
		bool download( std::string address, std::string port,
			const byte_http* raw_send, size_t size_send,
			decltype(end_function_) lp_func,
			std::vector<byte_http>* raw_recv)
		{
			if ( iocp_server_ptr_ == nullptr )
				return false; // should crash me. HOHOHO!
			if ( address != address_ || port != port_ )
			{
				if ( socket_ptr_ != nullptr )
				{
					socket_ptr_->clear();
					socket_ptr_ = nullptr;
				}
				address_ = address;
				port_ = port;
			}
			

			buffer_send_.resize(size_send);
			std::copy( raw_send, raw_send + size_send, buffer_send_.begin());
			buffer_recv_.clear();

			end_function_ = lp_func;
			
			boost::mutex::scoped_lock locker(thread_wait_mutex);
			if ( socket_ptr_ == nullptr )
			{
				socket_ptr_ = iocp_server_ptr_->create_connect( address, port,
					&http::downloader<Connection_Ptr,byte_http>::handle_connect, shared_from_this() );
			}
			else
			{
				handle_connect( socket_ptr_, boost::system::error_code() );
			}

			if ( raw_recv != NULL )
			{
				thread_wait_condition.wait(locker);
				raw_recv->swap( buffer_recv_  );

				buffer_recv_.clear();
			}

			return true;
		};
	public:
		void handle_connect( Connection_Ptr socket_ptr_, const boost::system::error_code& e )
		{
			if ( e || iocp_server_ptr_ == nullptr )
			{
				Debug_log( "%s (%s): %d %s\n", __FILE__,__LINE__,e.value(), e.message().c_str() );
				thread_wait_condition.notify_all();
				socket_ptr_->clear();
				socket_ptr_ = nullptr;
				return ;
			}

			iocp_server_ptr_->send( socket_ptr_, &buffer_send_[0], buffer_send_.size(), 
				&downloader::handle_send, shared_from_this() );
		};

		void handle_send( Connection_Ptr socket_ptr_, const boost::system::error_code& e, size_t bytes_transferred_ )
		{
			if ( e || iocp_server_ptr_ == nullptr )
			{
				Debug_log( "%s (%s): %d %s\n", __FILE__,__LINE__,e.value(), e.message().c_str() );
				thread_wait_condition.notify_all();
				socket_ptr_->clear();
				socket_ptr_ = nullptr;
				return ;
			}

			if ( bytes_transferred_ < buffer_send_.size() )
			{
				buffer_send_.erase( buffer_send_.begin(), buffer_send_.begin() + bytes_transferred_ );
				iocp_server_ptr_->send( socket_ptr_, &buffer_send_[0], buffer_send_.size(), 
				&downloader::handle_send, shared_from_this() );
			}
			iocp_server_ptr_->recv( socket_ptr_, HTTP_ONE_PACKET_SIZE, &downloader::handle_recv, shared_from_this() );
		};

		void handle_recv( Connection_Ptr ptr, const boost::system::error_code& e, size_t bytes_transferred_ )
		{
			if ( e || iocp_server_ptr_ == nullptr )
			{
				Debug_log( "%s (%s): %d %s\n", __FILE__,__LINE__,e.value(), e.message().c_str() );
				thread_wait_condition.notify_all();
				socket_ptr_->clear();
				socket_ptr_ = nullptr;
				return ;
			}
			if ( bytes_transferred_ > 0 )
				buffer_recv_.insert(buffer_recv_.end(), ptr->buff_temp().begin(),ptr->buff_temp().begin() + bytes_transferred_ );

			if (  bytes_transferred_ == HTTP_ONE_PACKET_SIZE )
			{
				iocp_server_ptr_->recv( socket_ptr_, HTTP_ONE_PACKET_SIZE, &downloader::handle_recv, shared_from_this() );
				return ;
			}
			else if ( bytes_transferred_ == 0 )
			{
				if ( false == end_function_.empty() )
				{
					end_function_( buffer_recv_ );
					end_function_.clear();
				}
				thread_wait_condition.notify_all();	
				return ;
			}
			else if ( bytes_transferred_ < HTTP_ONE_PACKET_SIZE )
			{
				// to be continue check package finished
				// head  \n\n or \r\n\r\n: continue recv
				// chunked 0\r\n\r\n: end
				// other: end
				bool finished = http_tools::http_reponse_check(buffer_recv_);
				if ( finished )
				{
					if ( false == end_function_.empty() )
					{
						end_function_( buffer_recv_ );
						end_function_.clear();
					}
					thread_wait_condition.notify_all();	
					return ;
				}
				iocp_server_ptr_->recv( socket_ptr_, HTTP_ONE_PACKET_SIZE, &downloader::handle_recv, shared_from_this() );
				return ;
			}

			// memory err
			thread_wait_condition.notify_all();	
			return ;
		};
	};
	
};
