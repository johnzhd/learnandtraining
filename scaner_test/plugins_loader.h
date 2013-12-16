#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
// As dll\so export

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace plugins_loader
{
	class plugins_server
	{
	public:
		plugins_server();
		~plugins_server();
	public:
		bool init();
		void clear();

	public:
		// html parser
		size_t html_parser(const char* body, std::vector<std::string>& v_out);
	};

	
	
};

namespace plugins_loader
{
	typedef boost::shared_ptr<plugins_server> server_ptr;
	server_ptr get_server();
};


