#pragma once

#include <string>
#include <map>
#include <vector>

#include <boost/function.hpp>


#include "task_data.h"

namespace task_http
{

	std::string control_download( IN task_data::task_start_struct_ptr base_ptr, IN task_data::task_url_struct_ptr url_ptr,
		OUT std::string& cookie, OUT std::vector<task_data::net_pack_ptr>& net_stream);

	std::string control_parser( IN task_data::task_map_ptr base_ptr, IN task_data::task_url_struct_ptr url_ptr );
};

