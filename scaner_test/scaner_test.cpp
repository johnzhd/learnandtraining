// scaner_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Windows.h>

#include <boost/thread.hpp>

#include "system.h"

#include "task_work.h"

#include <string>


// http://blog.csdn.net/qinlicang/article/details/5734298
// to resolve the memory problem


std::string start_task;

void function_start(bool b)
{
	if ( b )
	{
		Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call task_work::start in thread");
		std::string str;
		str = start_task;
		boost::function<void ()> func = [=]()->void{ Function_log("[start] %s.\n", task_work::start(start_task,"",""));};
		boost::thread io_server_thread(func);
	};
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::map<std::string,std::string> map_temp;

	if ( false == system_api::shell_code(argc, argv, map_temp, true) || map_temp["-target"].empty() )
	{
		printf( "Err! Unkown command. e.g.\n>scaner_test -target \"http://www.rising.com.cn\" -log \"normal\"\n" );
		system("pause");
		return 0;
	};
	

	std::string str;
	str = map_temp["-log"];
	if ( str.empty() )
		str = "normal";

	if ( str == "normal" )
	{
		system_log::set_log_show_type(system_log::Function);
	}
	else if ( str == "debug" )
	{
		system_log::set_log_show_type(system_log::Debug);
	}
	else if ( str == "high" )
	{
		system_log::set_log_show_type(system_log::High_Err);
	}
	else if ( str == "warning" )
	{
		system_log::set_log_show_type(system_log::Warning);
	}
	else if ( str == "noise" )
	{
		system_log::set_log_show_type(system_log::Noise);
	}
	else if ( str == "silence" )
	{
		system_log::set_log_show_type(system_log::Silence);
	}

	Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call function_start in thread");
	
	start_task = map_temp["-target"];

	str = system_api::run(function_start);

	Noise_log("[%s](%d) system_api::run return %s\n", __FUNCTION__, __LINE__, str);

	system("pause");
	return 0;
}

