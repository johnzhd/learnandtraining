#pragma once

#include <string>
#include <vector>

namespace file_base_tools
{
	std::string get_app_path();

	bool load_file( const char * file_name, std::vector<char>& file_buff );
}
