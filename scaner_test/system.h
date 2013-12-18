#pragma once

#include <string>

#include <boost/function.hpp>

namespace system_api
{
	std::string run( boost::function<void (bool)> func_init_finished, bool bReset = false);
};

