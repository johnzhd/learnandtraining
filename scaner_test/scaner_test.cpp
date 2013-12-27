// scaner_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Windows.h>

#include <boost/thread.hpp>

#include "system.h"

#include "task_data.h"
#include "task_work.h"

#include <string>


// http://blog.csdn.net/qinlicang/article/details/5734298
// to resolve the memory problem


task_data::task_start_struct_ptr g_start_ptr = nullptr;

void function_start(bool b)
{
	if ( b )
	{
		Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call task_work::start in thread");
		std::string str;
		if ( g_start_ptr == nullptr )
		{
			assert(false);
			return ;
		};
		boost::function<void ()> func = [=]()->void{ Function_log("[start] %s.\n", task_work::start(g_start_ptr));};
		boost::thread io_server_thread(func);
	};
	
}

void set_log( std::string str )
{
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
};

void set_start( std::map<std::string,std::string>& map_temp )
{
	std::string str;
	str = map_temp["-target"];
	if ( str.empty() )
	{
		g_start_ptr == nullptr;
		return ;
	}
	g_start_ptr.reset( new task_data::task_start_struct(str, map_temp["-head"], map_temp["-trigger"]) );

	str = map_temp["-domainlevel"];
	if ( false == str.empty() )
	{
		g_start_ptr->domain_sub_level_min = atoi( str.c_str() );
	};

};

int _tmain(int argc, _TCHAR* argv[])
{
	std::map<std::string,std::string> map_temp;
	std::string str_temp;

	if ( false == system_api::shell_code(argc, argv, map_temp, true) || map_temp["-target"].empty() )
	{
		printf( "Err! Unkown command. e.g.\n>scaner_test -target \"http://www.rising.com.cn\" -log \"normal\"\n" );
		system("pause");
		return 0;
	};
	

	set_log( map_temp["-log"]);

	set_start( map_temp );

	if ( g_start_ptr == nullptr )
	{
		printf( "Err! No task infomation. e.g.\n>scaner_test -target \"http://www.rising.com.cn\" -log \"normal\"\n" );
		system("pause");
		return 0;
	};


	Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call function_start in thread");


	str_temp = system_api::run(function_start);

	Noise_log("[%s](%d) system_api::run return %s\n", __FUNCTION__, __LINE__, str_temp);

	system("pause");
	return 0;
}

