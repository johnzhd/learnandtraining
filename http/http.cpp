#include "stdafx.h"
#include "http.hpp"

#include "http_tools.hpp"

#include "iocp_server.hpp"
#include <boost/function.hpp>


namespace http
{

bool download_one( bool post, std::string url, std::string head,
				  std::vector<unsigned char>& send_raw, std::vector<unsigned char>& recv_raw )
{
	std::string domain,port;

	url = http_tools::format_url(url);

	head = http_tools::format_head(head);

	domain = http_tools::get_domain(url);

	port = http_tools::get_port(url);

	if ( send_raw.empty() )
	{
		std::string str;
		str = http_tools::get_package( post?http_tools::POST:http_tools::GET, url, head );
		send_raw.resize( str.length() );
		std::copy(str.begin(),str.end(),send_raw.begin());
	}


	bool b = false;
#ifdef USE_SSL
	if ( http_tools::is_ssl( http_tools::get_protocal( url ) ) )
	{
		boost::shared_ptr<downloader<connection_ssl, unsigned char>> ptr_(new downloader<connection_ssl, unsigned char>() );
		boost::function<void (std::vector<unsigned char>&)> func;
		b = ptr_->download( domain, port,
			&send_raw[0], send_raw.size(),
			func, &recv_raw);
	}
	else
#endif
	{
		//
		boost::shared_ptr<downloader<connection_ptr, unsigned char>> ptr_(new downloader<connection_ptr, unsigned char>() );
		boost::function<void (std::vector<unsigned char>&)> func;
		b = ptr_->download( domain, port,
			&send_raw[0], send_raw.size(),
			func, &recv_raw);
	}
	return b;
}


};





