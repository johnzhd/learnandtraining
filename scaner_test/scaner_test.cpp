// scaner_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Windows.h>

#include <boost/thread.hpp>

#include "system.h"
#include "task_api.h"

#include <string>

std::string start_task;

void function_start(bool b)
{
	if ( b )
	{
		Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call task_work::start in thread");
		boost::thread io_server_thread(boost::bind(task_work::start,start_task,"",""));
	};
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	system_log::set_log_show_type(system_log::Noise);
	if ( argc < 2 )
	{
		printf( " No scan main url info. \n>scaner_test http://www.rising.com.cn\n" );
		return 0;
	};
	start_task = argv[1];

	Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call function_start in thread");
	
	std::string str;
	str = system_api::run(function_start);

	Noise_log("[%s](%d) system_api::run return %s\n", __FUNCTION__, __LINE__, str);


	return 0;
}

