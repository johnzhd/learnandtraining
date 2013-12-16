// scaner_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Windows.h>

#include <boost/thread.hpp>

#include "system.h"
#include "task_api.h"


void function_start()
{
	std::string str;
	str = system_api::run();

	High_log("Quit from system run. %s\n",str.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	system_log::set_log_show_type(system_log::Debug);

	(argc);
	(argv);
	boost::thread io_server_thread(function_start);

	// do not use : t.join();

	Sleep(20);


	
	io_server_thread.join();




	return 0;
}

