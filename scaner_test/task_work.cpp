#include "stdafx.h"
#include "task_work.h"

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>


#include <sstream>

#include "task_data.h"
#include "thread_pool.h"
#include "policy_work.h"

#include "charset.h"
#include "http.hpp"

#include "plugins_loader.h"

namespace task_work
{
	std::string make_err_url( std::string url )
	{
		std::string str[4];
		str[3].clear();
		if ( false == http_tools::get_url_info(url, &str[0],&str[1],&str[2],NULL,NULL) )
			return str[3];

		str[3] = str[0] + "://" + str[1] + ":" + str[2] + "/nowaytobeexistpage301.html";
		return str[3];
	};

	inline bool push_into_thread_pool(function_type func)
	{
		thread_pool_api::thread_pool_ptr thread_pool_point;
		thread_pool_point = thread_pool_api::get_pool();
		assert( thread_pool_point );
		if ( thread_pool_point == false )
			return false;

		return thread_pool_point->push( func );
	};

	std::string start( std::string url, std::string head, std::string trigger )
	{
		task_data::task_map_list_ptr task_map_list_point;

		task_map_list_point = task_data::get_globle_task_map();

		if ( task_map_list_point == nullptr )
		{
			return system_log::err_ret( system_log::system_err(1) );
		};

		// new struct
		task_data::task_map_ptr map_ptr(new task_data::task_map(task_data::task_start_struct_ptr(new task_data::task_start_struct(url,head,trigger))));

		if ( false == task_map_list_point->insert_new( map_ptr ) )
		{
			return system_log::err_ret( system_log::op_err(1) );
		}

		// download main page
		std::vector<unsigned char> send_raw,recv_raw;
		if ( false == http::download_one(false, url, head, send_raw, recv_raw) )
		{
			return system_log::err_ret( system_log::op_err(2) );
		};
		std::vector<http_tools::http_info> v_main,v_err;
		if ( false == http_tools::http_reponse_completed(recv_raw, v_main) )
		{
			return system_log::err_ret( system_log::op_err(3) );
		};
		bool b = false;
		for ( auto& h : v_main )
		{
			if ( h.status == 200 && crawl_page(h.translate_wstr_body) > 0 )
			{
				b = true;
			};
		}
		if ( b == false )
			return system_log::err_ret( system_log::op_err(4) );


		// download failed page
		map_ptr->err_page_ptr.reset(new task_data::task_err_page());
		send_raw.clear();
		recv_raw.clear();
		if (true == http::download_one(false, make_err_url(url), head, send_raw, recv_raw)
			&& true == http_tools::http_reponse_completed(recv_raw, v_err)
			&& false == v_err.empty())
		{
			auto a = v_err.rbegin();
			if ( a->status >= 300 )
			{
				map_ptr->err_page_ptr->set_err_page( task_data::Basic_404 );
			}
			else
			{
				map_ptr->err_page_ptr->set_err_page( task_data::Self_200, a->translate_wstr_body );
			}
		}
		else 
		{
			map_ptr->err_page_ptr->set_err_page( task_data::Basic_404 );
		}

		
		// start thread pool working
		task_data::task_url_struct_ptr url_point(new task_data::task_url_struct(url));
		url_point->b_post = false;
		url_point->running_trigger = task_data::task_start_struct::format_trigger(trigger + "||domain");
		if ( false == map_ptr->insert_new_url(url_point) ) // first url set trigger add "domain"
		{
			return system_log::err_ret( system_log::op_err(5) );
		};

		if ( false == push_into_thread_pool( boost::bind(task_work::pull_and_download_url,map_ptr) ) )
		{
			return system_log::err_ret( system_log::op_err(5) );
		}
		return system_log::success_ret();
	}


	void pull_and_download_url(task_data::task_map_ptr map_ptr)
	{
		task_data::task_url_struct_ptr url_point;
		size_t url_no = 0;
		//Empty list return
		if ( false == map_ptr->fresh_one_url(url_no,url_point) )
			return ;

		

		if ( map_ptr->is_url_empty() )
		{
			push_into_thread_pool(boost::bind(task_work::pull_and_download_url,map_ptr));
		};

		try
		{
			url_point->first_send.clear();
			if ( false == http::download_one(url_point->b_post, url_point->url_origin, map_ptr->start_ptr->running_head,
				url_point->first_send, url_point->first_recv) )
			{
				throw std::string("start download failed.");
			};
			std::vector<http_tools::http_info> v_list;
			if ( false == http_tools::http_reponse_completed(url_point->first_recv, v_list) )
			{
				throw std::string("download failed. unable to analyse http protocal.");
			};

			std::vector<std::string> url_list;
			url_point->translate_body = L"";
			for ( auto& http_package : v_list )
			{
				url_list.clear();
				if ( false == http_package.translate_wstr_body.empty() && crawl_page(http_package.translate_wstr_body,&url_list) > 0 )
					map_ptr->insert_new_url( url_list );

				url_point->translate_body += http_package.translate_wstr_body;
			};

			policy_api::policy_server_ptr policy_point;
			policy_point = policy_api::get_server();
			if ( policy_point == nullptr )
				return ;
			std::vector<policy_api::policy_work_ptr> policy_list;
			if ( false == policy_point->get_policys( url_point->running_trigger,policy_list) || policy_list.size() == 0 )
				return ;

			url_point->running_thread = policy_list.size();

			for( auto ptr : policy_list )
			{
				push_into_thread_pool(boost::bind(task_work::run_policy, map_ptr, url_no, ptr) );
			};
		}
		catch ( std::string& str )
		{
			Noise_log( "%s %s %s \n", __FILE__, __LINE__, str );

			map_ptr->finished_one_url(url_point);
			url_point = NULL;

			return ;
		};
		
	};



	size_t crawl_page( std::vector<wchar_t>& body, std::vector<std::string>* url_list )
	{
		return crawl_page(charset::to_acsII(body),url_list);
	};
	size_t crawl_page( const std::wstring& body, std::vector<std::string>* url_list )
	{
		return crawl_page(charset::to_acsII(body),url_list);
	};
	size_t crawl_page( const std::string& body, std::vector<std::string>* url_list )
	{
		size_t n = 0;
		std::vector<std::string> v;
		v.clear();
		n = plugins_loader::get_server()->html_parser(body.c_str(),v);
		
		if ( nullptr != url_list )
			url_list->swap(v);
		return n;
	};









	void run_policy(task_data::task_map_ptr map_ptr, size_t url_no, policy_api::policy_work_ptr policy_ptr)
	{
		task_data::task_url_struct_ptr url_ptr;
		std::vector<std::string> in, out;
		if ( false == map_ptr->get_one_url(url_no,url_ptr) )
			return ;

		out.resize(3,"");
		in.resize(3,"");
		in[0] = url_ptr->url_origin;
		in[1].assign(url_ptr->first_recv.begin(),url_ptr->first_recv.end());
		in[2] = charset::to_acsII(url_ptr->translate_body);
		if ( true == policy_ptr->call_function( out, "Init_Lua", in ) )
		{
			/*if ( find vul )
			{
			task_data::task_result_struct_ptr ptr(new task_data::task_result_struct(out[0],policy_ptr->get_vul_id(),out[1],out[2],false));
			map_ptr->insert_result( url_no,ptr);
			}*/
		}
		// if ( policy finished )
		map_ptr->finished_one_url(url_no);
		// to be continue here
		//else
		//run_xxxx;

	}
























};