#pragma once

#include <string>
#include <map>
#include <boost/function.hpp>


namespace system_api
{
	std::string run( boost::function<void (bool)> func_init_finished, bool bReset = false);

	bool shell_code( int argc, _TCHAR* argv[], std::map<std::string,std::string>& map_out, bool b_transform_lower = false );
};

