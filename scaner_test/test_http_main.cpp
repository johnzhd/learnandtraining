// scaner_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include "globle_api.h"

#include <boost/thread.hpp>

#include "http.hpp"


int _tmain(int argc, _TCHAR* argv[])
{
	(argc);
	(argv);
	globle_api ga;
	boost::thread t(boost::bind(&globle_api::run,&ga));

	// do not use : t.join();

	Sleep(20);

	std::vector<unsigned char> raw;

	if ( false == http::download_one( false, "http://www.baidu.com","",raw) )
	{
		return 0;
	}

	std::vector<http_tools::http_info> v;
	if ( false == http_tools::http_reponse_completed( raw,v) )
	{
		return 0;
	};

	size_t n;
	n = v.size();
	const char *p, *p1;

	for ( auto it = v.begin(); it != v.end(); it++ )
	{
		n = it->status;
		for ( auto j = it->head.begin(); j != it->head.end(); j++ )
		{
			p = j->first.c_str();
			p1 = j->second.c_str();
		}

		p = reinterpret_cast<const char*>(&it->body[0]);
		p = NULL;
		p = reinterpret_cast<const char*>(&it->package_origin[0]);
		p = NULL;
	}


	return 0;
}

