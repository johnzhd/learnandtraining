#pragma once

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/enable_shared_from_this.hpp>



// define socket types
typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> listenor_ptr;

#include "connect_socket.h"

// define handle types
typedef boost::function<void(connection_ptr connection_, const boost::system::error_code& e)> func_handle;
typedef boost::function<void(connection_ptr connection_, const boost::system::error_code& e, size_t bytes_transferred)> func_handle_bytes;

#ifdef USE_SSL

#include "connect_ssl.h"


typedef boost::function<void(connection_ssl connection_, const boost::system::error_code& e)> func_ssl_handle;
typedef boost::function<void(connection_ssl connection_, const boost::system::error_code& e, size_t bytes_transferred)> func_ssl_handle_bytes;

#endif 

#define IOCP_SERVER_POOL_SIZE	(2)





//////////////////////////////////////////////////////////////////////////////////////////////
//iocp_server_pool
//


class iocp_server_pool
	: private boost::noncopyable
{
public:
	/// Construct the io_service pool.
	explicit iocp_server_pool(std::size_t pool_size);
	~iocp_server_pool(void);




	/// Run all io_service objects in the pool.
	void run();

	/// Stop all io_service objects in the pool.
	void stop();

	/// Get an io_service to use.
	boost::asio::io_service& get_io_service();

private:
	typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
	typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;

	/// The pool of io_services.
	std::vector<io_service_ptr> io_services_;

	/// The work that keeps the io_services running.
	std::vector<work_ptr> work_;

	/// The next io_service to use for a connection.
	std::size_t next_io_service_;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//iocp_server_api
//


class iocp_server
	: public boost::enable_shared_from_this<iocp_server>,
	private boost::noncopyable
{
public:
	explicit iocp_server(void);
	~iocp_server(void);
public:
	void run();
	void stop();
private:
	void handle_stop();
private:
	template< typename Connection_Ptr>
	boost::asio::ip::tcp::endpoint translate_endpoint( Connection_Ptr connection_, const std::string& address, const std::string& port ){
		boost::asio::ip::tcp::resolver resolver(connection_->socket().get_io_service());
		boost::asio::ip::tcp::resolver::query query(address, port);
		boost::asio::ip::tcp::endpoint endpoint_;
		try {
			endpoint_ = *resolver.resolve(query);
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
		return endpoint_;
	};

	template< typename Connection_Ptr>
	boost::asio::ip::tcp::resolver::iterator translate_endpoint_iter( Connection_Ptr connection_, const std::string& address, const std::string& port ){
		boost::asio::ip::tcp::resolver resolver(connection_->socket().get_io_service());
		boost::asio::ip::tcp::resolver::query query(address, port);
		boost::asio::ip::tcp::resolver::iterator iterator;
		try {
			iterator = resolver.resolve(query);
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
		return iterator;
	};
public:
	/////////// send //////////////////////////////////////////////////
	template< typename Connection_Ptr, typename BYTE_Ptr, typename FunC_BYTE>
	void send( Connection_Ptr connection_, const BYTE_Ptr bytes_, size_t size_
		, FunC_BYTE func_send_)
	{
		boost::asio::async_write(connection_->socket(),
			boost::asio::buffer(bytes_,size_),
			boost::bind(func_send_,
			connection_,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	template< typename Connection_Ptr, typename BYTE_Ptr, typename FunC_BYTE, typename T1>
	void send( Connection_Ptr connection_, const BYTE_Ptr bytes_, size_t size_
		, FunC_BYTE func_send_, T1 t1)
	{
		boost::asio::async_write(connection_->socket(),
			boost::asio::buffer(bytes_,size_),
			boost::bind(func_send_,t1,
			connection_,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	/////////// recv //////////////////////////////////////////////////
	template< typename Connection_Ptr, typename FunC_BYTE>
	void recv( Connection_Ptr connection_, size_t size_
		, FunC_BYTE func_recv_)
	{

		connection_->socket().async_read_some( size_ == 0 ? (boost::asio::buffer(connection_->buff_temp())): (boost::asio::buffer(connection_->buff_temp(),size_)),
			boost::bind(func_recv_,
			connection_,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	template< typename Connection_Ptr, typename FunC_BYTE, typename T1>
	void recv( Connection_Ptr connection_, size_t size_
		, FunC_BYTE func_recv_, T1 t1)
	{

		connection_->socket().async_read_some( size_ == 0 ? (boost::asio::buffer(connection_->buff_temp())): (boost::asio::buffer(connection_->buff_temp(),size_)),
			boost::bind(func_recv_,t1,
			connection_,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
public:
	// set listen acceptor

	listenor_ptr create_listen( const std::string& address, const std::string& port);

	/////////// accept //////////////////////////////////////////////////
	template< typename FunC >
	void start_accept( listenor_ptr acceptor_, FunC func_accept_ )
	{
		connection_ptr new_connection_;
		new_connection_.reset(new connect_socket(iocp_server_pool_.get_io_service()));
		acceptor_->async_accept(new_connection_->socket(),
			boost::bind(func_accept_,
			new_connection_,
			boost::asio::placeholders::error));
	}
	template< typename FunC, typename T1 >
	void start_accept( listenor_ptr acceptor_, FunC func_accept_, T1 t1 )
	{
		connection_ptr new_connection_;
		new_connection_.reset(new connect_socket(iocp_server_pool_.get_io_service()));
		acceptor_->async_accept(new_connection_->socket(),
			boost::bind(func_accept_,t1,
			new_connection_,
			boost::asio::placeholders::error));
	}

	/////////// connect //////////////////////////////////////////////////
	template< typename FunC >
	connection_ptr create_connect( const std::string& address, const std::string& port
		, FunC func_connect_)
	{
		connection_ptr new_connection_;
		new_connection_.reset( new connect_socket(iocp_server_pool_.get_io_service()));
		new_connection_->socket().async_connect(translate_endpoint(new_connection_, address, port ),
			boost::bind(func_connect_,
			new_connection_,
			boost::asio::placeholders::error));

		return new_connection_;
	};

	template< typename FunC, typename T1 >
	connection_ptr create_connect( const std::string& address, const std::string& port
		, FunC func_connect_, T1 t1)
	{
		connection_ptr new_connection_;
		new_connection_.reset( new connect_socket(iocp_server_pool_.get_io_service()));
		new_connection_->socket().async_connect(translate_endpoint(new_connection_, address, port ),
			boost::bind(func_connect_,t1,
			new_connection_,
			boost::asio::placeholders::error));

		return new_connection_;
	};



public:
	iocp_server_pool iocp_server_pool_;
	boost::asio::signal_set signals_;



	//////////////////////////////////////// SSL ///////////////////////////////////////////

#ifdef USE_SSL
public:

	boost::asio::ssl::context context_server_;
	boost::asio::ssl::context context_client_;
protected:
	// init //
	std::string get_password() const
	{
		return "test";
	}
	/// handshake ///
	void handshake( bool bServer, connection_ssl new_connection_, const boost::system::error_code& e, func_ssl_handle func_handshake_ )
	{
		if ( e )
		{
			if ( func_handshake_ )
			{
				func_handshake_(new_connection_,e);
				return ;
			}
		}
		new_connection_->socket().async_handshake( (bServer ? boost::asio::ssl::stream_base::server : boost::asio::ssl::stream_base::client),
			boost::bind(func_handshake_,
			new_connection_,
			boost::asio::placeholders::error));
	};

	/// client ///
	/// create ///
	bool client_verify_callback( bool preverified, boost::asio::ssl::verify_context& context_ ){
		// do nothing
		return true; 
		// But, We could do somethings, if needed.
		//char subject_name[256];
		//X509* cert = X509_STORE_CTX_get_current_cert(context_.native_handler());
		//X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		//printf( "%s", subject_name);
		// return preverified;
	}
public:
	void init_context( std::string str_client_verify_file,
		std::string str_server_key_, std::string str_server_certificate_chain_file,
		std::string str_server_private_key_file, std::string str_server_tmp_dh_file )
	{
		boost::system::error_code err;
		/// client ///
		context_client_.load_verify_file( str_client_verify_file,err);

		/// server ///
		context_server_.set_options( boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::single_dh_use
			, err);


		context_server_.set_password_callback( boost::bind(&iocp_server::get_password,this), err);
		context_server_.use_certificate_chain_file("server.pem",err);
		context_server_.use_private_key_file("server.pem", boost::asio::ssl::context::pem, err);
		context_server_.use_tmp_dh_file("dh512.pem", err);
	};



	/// server ///
	/// accept ///
	void start_accept_ssl( listenor_ptr acceptor_, func_ssl_handle func_accept_ )
	{
		connection_ssl new_connection_;
		new_connection_.reset(new connect_ssl(iocp_server_pool_.get_io_service(), context_server_));
		acceptor_->async_accept(new_connection_->socket().lowest_layer(),
			boost::bind(&iocp_server::handshake, shared_from_this(), true, 
			new_connection_,
			boost::asio::placeholders::error,
			func_connect_));
	};
	// client //
	connection_ssl create_connect_ssl( const std::string& address, const std::string& port
		, func_ssl_handle func_connect_)
	{
		// Use boost::system::error_code to avoid boost::asio::detail::throw_error
		connection_ssl new_connection_;
		boost::system::error_code err;
		new_connection_.reset( new connect_ssl(iocp_server_pool_.get_io_service(), context_client_));
		new_connection_->socket().set_verify_mode(boost::asio::ssl::verify_peer,err);
		new_connection_->socket().set_verify_callback( boost::bind( &iocp_server::client_verify_callback, this), err);
		boost::asio::async_connect( new_connection_->socket().lowest_layer(), translate_endpoint_iter(new_connection_, address, port),
			boost::bind(&iocp_server::handshake, shared_from_this(), false, 
			new_connection_,
			boost::asio::placeholders::error,
			func_connect_));

		return new_connection_;
	};


	
#endif //#ifdef USE_SSL
};




namespace iocp_server_api
{
	typedef boost::shared_ptr<iocp_server> iocp_server_ptr;
	iocp_server_ptr get_server(); // get union server
};