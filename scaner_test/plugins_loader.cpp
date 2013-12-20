#include "stdafx.h"
#include "plugins_loader.h"

#include "file_base_tools.h"

namespace plugins_loader
{
#ifdef _MSC_VER
	typedef HMODULE handle_library;
#else
	typedef void* handle_library;
#endif

	std::vector<handle_library> vh_plugins;


	


	handle_library open_library(std::string fname)
	{
		std::string str_temp;
#ifdef _MSC_VER
		str_temp = file_base_tools::get_app_path()+fname+".dll";
		return ::LoadLibrary( str_temp.c_str() );
#else
		str_temp = file_base_tools::get_app_path()+fname+".so";
		return dlopen(str_temp.c_str(), RTLD_LAZY);
#endif
		
	};


	void* load_func( handle_library h, const char * pname )
	{
		void* func;
		assert(h);
#ifdef _MSC_VER
		func = ::GetProcAddress(h,pname);
#else
		func = dlsym(h, pname);
		pszErr = dlerror();
		if ( pszErr != nullptr )
		{
			High_log("%s %s %s\n", __FILE__, __LINE__, pszErr);
			func = nullptr;
		}
#endif
		return func;
	};



	void close_library( handle_library h )
	{
		assert(h);
#ifdef _MSC_VER
		CloseHandle(h);
#else
		dlclose(h);
#endif
	};

	typedef size_t (*func_html_parser_run)(const std::string& body, std::set<std::string>& v_out, std::string page_url);
	func_html_parser_run call_func_html_parser_run = nullptr;
	inline static bool init_html_parser()
	{
		auto h_temp = open_library("html_parser");
		if ( h_temp == nullptr )
			return false;

		call_func_html_parser_run = (func_html_parser_run)load_func(h_temp, "API_html_parser");
		if ( call_func_html_parser_run == nullptr )
		{
			close_library(h_temp);
			return false;
		}
		vh_plugins.push_back(h_temp);
		return true;
	}
	size_t plugins_server::html_parser(const std::string& body, std::set<std::string>& v_out, std::string page_url)
	{
		if ( call_func_html_parser_run )
			return call_func_html_parser_run(body,v_out,page_url);
		return 0;
	}




	plugins_server::plugins_server()
	{
		assert(vh_plugins.empty());
	};

	plugins_server::~plugins_server()
	{
	};

	bool plugins_server::init()
	{
		// load html parser
		if( false == init_html_parser() )
		{
			clear();
			return false;
		}

		return true;
	};

	void plugins_server::clear()
	{
		for( auto h : vh_plugins)
		{
			if ( nullptr != h )
				close_library(h);
		};
		vh_plugins.clear();
	};


	






















};

namespace plugins_loader
{
	server_ptr g_ptr=nullptr;
	server_ptr get_server()
	{
		if ( g_ptr == nullptr )
			g_ptr.reset(new plugins_server());
		if ( g_ptr->init() == false )
		{
			g_ptr = nullptr;
		};
		return g_ptr;
	}
};