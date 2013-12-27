#include "stdafx.h"
#include "plugins_loader.h"

#include "file_base_tools.h"

#include "html_parser_api.h"



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

};







////////////////////////////////////////////////////////////////////////////////////////
// plugins
//

namespace plugins_loader
{
};




//
// plugins
////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
// System
//


namespace plugins_loader
{

	

	plugins_server::plugins_server()
	{
		assert(vh_plugins.empty());

		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};

	plugins_server::~plugins_server()
	{
		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};

	bool plugins_server::init()
	{
		// load html parser
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