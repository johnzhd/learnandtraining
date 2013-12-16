#include "stdafx.h"
#include "file_base_tools.h"

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
}