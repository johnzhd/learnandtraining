#include "stdafx.h"
#include "file_base_tools.h"

#include <iostream>
#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#else
#endif

namespace file_base_tools
{
	std::string g_app_path("");
	std::string get_app_path()
	{
		if ( g_app_path.empty() )
		{
			char path[_MAX_PATH + 2] = {0};
			int n;
#ifdef _MSC_VER
			n = GetModuleFileName( NULL, path, _MAX_PATH );
#else
			n = readlink( "/proc/self/exe", result, PATH_MAX );
			n = (n>0?n:0);
#endif
			path[n] = '\0';
			g_app_path = path;
			auto npos = g_app_path.find_last_of("\\/");
			if ( npos != std::string::npos )
				g_app_path.erase( npos + 1 );
		}
		return g_app_path;
	}


	bool load_file( const char * file_name, std::vector<char>& file_buff )
	{
		std::ifstream f( file_name, std::ios::_Nocreate | std::ios::binary );

		std::filebuf *ptr;

		if ( false == f.is_open() )
			return false;

		ptr = f.rdbuf();
		if ( ptr == nullptr )
		{
			f.close();
			return false;
		}


		std::streamoff size = ptr->pubseekoff(0, std::ios::end, std::ios::in);
		if ( size <= 0 )
		{
			f.close();
			return false;
		};

		ptr->pubseekpos(0,std::ios::in);

		file_buff.resize(static_cast<size_t>(size),0);
		ptr->sgetn(&file_buff[0],size);

		f.close();

		return true;
	};
}