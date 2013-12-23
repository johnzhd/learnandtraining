#include "stdafx.h"
#include "iocp_server.hpp"


#include <stdexcept>
#include <boost/thread/thread.hpp>



////////////////////////////////////////////////////////////////////////////////////
// iocp_server_pool


iocp_server_pool::iocp_server_pool(std::size_t pool_size)
	: next_io_service_(0)
{
	if (pool_size == 0)
		throw std::runtime_error("iocp_server_pool size is 0");

	// Give all the io_services work to do so that their run() functions will not
	// exit until they are explicitly stopped.
	for (std::size_t i = 0; i < pool_size; ++i)
	{
		io_service_ptr io_service(new boost::asio::io_service);
		work_ptr work(new boost::asio::io_service::work(*io_service));
		io_services_.push_back(io_service);
		work_.push_back(work);
	}

	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
}
iocp_server_pool::~iocp_server_pool(void){
	stop();
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
}
void iocp_server_pool::run()
{
	// Create a pool of threads to run all of the io_services.
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < io_services_.size(); ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&boost::asio::io_service::run, io_services_[i])));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

void iocp_server_pool::stop()
{
	// Explicitly stop all io_services.
	for (std::size_t i = 0; i < io_services_.size(); ++i)
		io_services_[i]->stop();
}

boost::asio::io_service& iocp_server_pool::get_io_service()
{
	// Use a round-robin scheme to choose the next io_service to use.
	boost::asio::io_service& io_service = *io_services_[next_io_service_];
	++next_io_service_;
	if (next_io_service_ == io_services_.size())
		next_io_service_ = 0;
	return io_service;
}

namespace iocp_server_api
{
	iocp_server_ptr g_io_ptr_ = NULL;

	iocp_server_ptr get_server()
	{
		if ( g_io_ptr_ == NULL )
		{
			g_io_ptr_.reset( new iocp_server() );
		}
		return g_io_ptr_;
	}
};

////////////////////////////////////////////////////////////////////////////////////
// iocp_server



iocp_server::iocp_server(void)
	:iocp_server_pool_(IOCP_SERVER_POOL_SIZE),
	signals_(iocp_server_pool_.get_io_service())
{
	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)
	signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
	signals_.async_wait(boost::bind(&iocp_server::handle_stop, this));

	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
}


iocp_server::~iocp_server(void)
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
}

void iocp_server::run(){
	iocp_server_pool_.run();
}

void iocp_server::stop()
{
	handle_stop();
}

void iocp_server::handle_stop(){
	iocp_server_pool_.stop();
}

listenor_ptr iocp_server::create_listen(const std::string& address, const std::string& port){
	listenor_ptr ptr_;
	ptr_.reset(new boost::asio::ip::tcp::acceptor(iocp_server_pool_.get_io_service()));
	boost::asio::ip::tcp::resolver resolver(ptr_->get_io_service());
	boost::asio::ip::tcp::resolver::query query(address, port);
	boost::asio::ip::tcp::endpoint local_endpoint_ = *resolver.resolve(query);
	ptr_->open(local_endpoint_.protocol());
	ptr_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	ptr_->bind(local_endpoint_);
	ptr_->listen();
	return ptr_;
}




