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

#include "task_http.h"

namespace task_work
{
	std::string make_err_url( std::string url )
	{
		std::string str[4];
		str[3].clear();
		if ( false == http_tools::get_url_info(url, &str[0],&str[1],&str[2],NULL,NULL) )
			return str[3];

		str[3] = str[0] + "://" + str[1] + str[2] + "/nowaytobeexistpage301.html";
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
		task_data::task_start_struct_ptr ptr(new task_data::task_start_struct(url,head,trigger));
		return start( ptr );
	}


	std::string start( task_data::task_start_struct_ptr start_ptr )
	{
		if ( start_ptr == nullptr )
			return system_log::err_ret( system_log::system_err(1) );
		
		start_ptr->format_start();

		task_data::task_map_list_ptr task_map_list_point;

		task_map_list_point = task_data::get_globle_task_map();

		if ( task_map_list_point == nullptr )
		{
			return system_log::err_ret( system_log::system_err(7) );
		};

		// new struct
		task_data::task_map_ptr map_ptr(new task_data::task_map(start_ptr));

		if ( false == task_map_list_point->insert_new( map_ptr ) )
		{
			return system_log::err_ret( system_log::op_err(1) );
		}
		Function_log("[%s] Check main url %s.\n", __FUNCTION__, start_ptr->main_url );
		Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call http::download_one main.");
		// download main page
		std::vector<unsigned char> send_raw,recv_raw;
		if ( false == http::download_one(false, start_ptr->main_url, start_ptr->base_head, send_raw, recv_raw) )
		{
			return system_log::err_ret( system_log::op_err(2) );
		};

		std::vector<http_tools::http_info> v_main,v_err;
		if ( false == http_tools::http_reponse_completed(recv_raw, v_main, start_ptr->base_trigger) )
		{
			return system_log::err_ret( system_log::op_err(3) );
		};

		
		bool b = false;
		for ( auto& h : v_main )
		{
			if ( h.status == 200 && crawl_page(reinterpret_cast<char*>(&h.body_origin[0]),h.body_origin.size(),nullptr,
				http_tools::get_page(start_ptr->main_url), start_ptr->domain_sub_level_min) > 0 )
			{
				b = true;
			};
		}
		if ( b == false )
			return system_log::err_ret( system_log::op_err(4) );


		// download failed page
		Noise_log("[%s](%d) %s\n", __FUNCTION__, __LINE__, "Call http::download_one errpage.");

		map_ptr->err_page_ptr.reset(new task_data::task_err_page());
		send_raw.clear();
		recv_raw.clear();
		if (true == http::download_one(false, make_err_url(start_ptr->main_url), start_ptr->running_head, send_raw, recv_raw)
			&& true == http_tools::http_reponse_completed(recv_raw, v_err,start_ptr->base_trigger)
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
		task_data::task_url_struct_ptr url_point(new task_data::task_url_struct(start_ptr->main_url));
		url_point->b_post = false;
		url_point->running_trigger = policy_api::trigger_base::format_trigger(start_ptr->base_trigger, "domain");
		if ( false == map_ptr->insert_new_url(url_point) ) // first url set trigger add "domain"
		{
			return system_log::err_ret( system_log::op_err(5) );
		};

		if ( false == push_into_thread_pool( boost::bind(task_work::pull_and_download_url,map_ptr) ) )
		{
			return system_log::err_ret( system_log::op_err(6) );
		}

		Function_log("[%s] Start task %s.\n", __FUNCTION__, map_ptr->get_task_union_id() );
		return system_log::success_ret();
	};

	size_t crawl_page( const char* p_body, size_t size_body, std::set<std::string>* url_list,
		const std::string& page_url, size_t domain_sub_level )
	{
		size_t n = 0;
		std::set<std::string> v;
		v.clear();
		n = plugins_loader::get_server()->html_parser(p_body, size_body, v, page_url,domain_sub_level);
		
		if ( nullptr != url_list )
			url_list->swap(v);
		return n;
	};

	void pull_and_download_url(task_data::task_map_ptr map_ptr)
	{
		task_data::task_url_struct_ptr url_point;
		size_t url_no = 0;
		//Empty list return
		if ( false == map_ptr->fresh_one_url(url_no,url_point) )
			return ;

		do
		{
			url_point->first_send.clear();
			Function_log("[%s] %d Download.\n", __FUNCTION__, url_no );
			if ( false == http::download_one(url_point->b_post, url_point->url_origin, map_ptr->start_ptr->running_head,
				url_point->first_send, url_point->first_recv) )
			{
				Noise_log( "[%s] %d download failed. \n", __FUNCTION__, url_no );
				break; //trick for return
			};
			std::vector<http_tools::http_info> v_list;
			if ( false == http_tools::http_reponse_completed(url_point->first_recv, v_list, url_point->running_trigger) )
			{
				Noise_log( "[%s] %d unzip failed. \n", __FUNCTION__, url_no );
				break; //trick for return
			};

			std::set<std::string> url_list;
			url_point->translate_body = L"";
			// for debug

			for ( auto& http_package : v_list )
			{
				url_list.clear();
				if ( false == http_package.body_origin.empty() &&
					crawl_page(reinterpret_cast<char*>(&http_package.body_origin[0]),http_package.body_origin.size(), &url_list, http_tools::get_page(url_point->url_origin)) > 0 )
				{
					map_ptr->insert_new_url( url_list );
				}

				url_point->translate_body += http_package.translate_wstr_body;
			};

			policy_api::policy_server_ptr policy_point;
			policy_point = policy_api::get_server();
			if ( policy_point == nullptr )
			{
				Noise_log( "%s %s %s \n", __FILE__, __LINE__, "get policy failed. unable to get policy server." );
				break; //trick for return
			}
			
			std::vector<std::string> policy_list;
			if ( false == policy_point->get_policys( url_point->running_trigger,policy_list) || policy_list.empty() )
			{
				Noise_log( "%s %s %s %s \n", __FILE__, __LINE__, "get policy failed. No triggered policy.", url_point->running_trigger );
				break; //trick for return
			}

			Noise_log("[%s] %d Start policy(%d).\n", __FUNCTION__, url_no, policy_list.size() );
			url_point->running_thread = policy_list.size() + 1; // 1 : download & crawl( this thread already done) + size() : policy need

			for( auto ptr : policy_list )
			{
				push_into_thread_pool(boost::bind(task_work::run_policy_name, map_ptr, url_no, ptr) );
			};
		}
		while ( url_no != url_no );  //trick for return // to fool warning as error check

		Function_log("[%s] finished %d\n", __FUNCTION__, url_no );

		map_ptr->finished_one_url(url_point);

		int n = map_ptr->url_list_remain();

		while ( n-- > 0 )
		{
			push_into_thread_pool(boost::bind(task_work::pull_and_download_url,map_ptr));
		};
	};

	void pull_and_work_url(task_data::task_map_ptr map_ptr)
	{
		task_data::task_url_struct_ptr url_point;
		size_t url_no = 0;

		//Empty list return
		if ( false == map_ptr->fresh_one_url(url_no,url_point) )
			return ;

		do
		{
			Function_log("[%s] %d Download.\n", __FUNCTION__, url_no );
			std::string str_cookie;
			if ( false == task_http::control_download( map_ptr->start_ptr, url_point, str_cookie, url_point->http_packages ) )
			{
				Noise_log( "[%s] %d download failed. \n", __FUNCTION__, url_no );
				break; //trick for return
			}

			map_ptr->start_ptr->update_running_cookie( str_cookie );

			if ( false == task_http::control_parser( map_ptr, url_point ))
			{
				Noise_log( "[%s] %d parser failed. \n", __FUNCTION__, url_no );
				break;
			};

			policy_api::policy_server_ptr policy_point;
			policy_point = policy_api::get_server();
			if ( policy_point == nullptr )
			{
				Noise_log( "%s %s %s \n", __FILE__, __LINE__, "get policy failed. unable to get policy server." );
				break; //trick for return
			}
			
			std::vector<std::string> policy_list;
			if ( false == policy_point->get_policys( url_point->running_trigger,policy_list) || policy_list.empty() )
			{
				Noise_log( "%s %s %s %s \n", __FILE__, __LINE__, "get policy failed. No triggered policy.", url_point->running_trigger );
				break; //trick for return
			}

			Noise_log("[%s] %d Start policy(%d).\n", __FUNCTION__, url_no, policy_list.size() );
			url_point->running_thread = policy_list.size() + 1; // 1 : download & crawl( this thread already done) + size() : policy need

			for( auto ptr : policy_list )
			{
				push_into_thread_pool(boost::bind(task_work::run_policy_name, map_ptr, url_no, ptr) );
			};
		}
		while ( url_no != url_no );  //trick for return // to fool warning as error check

		Function_log("[%s] finished %d\n", __FUNCTION__, url_no );

		map_ptr->finished_one_url(url_point);

		int n = map_ptr->url_list_remain();

		while ( n-- > 0 )
		{
			push_into_thread_pool(boost::bind(task_work::pull_and_download_url,map_ptr));
		};
	};

	








	void run_policy(task_data::task_map_ptr map_ptr, size_t url_no, policy_api::policy_work_ptr policy_ptr)
	{
		task_data::task_url_struct_ptr url_ptr;
		std::vector<std::string> in, out;
		if ( false == map_ptr->get_one_url(url_no,url_ptr) )
			return ;

		//Noise_log("[%s](%d) Policy %s | %s\n", __FUNCTION__, __LINE__, policy_ptr->name() , url_ptr->url_origin.c_str() );

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

	void run_policy_name(task_data::task_map_ptr map_ptr, size_t url_no, std::string policy_name)
	{
		task_data::task_url_struct_ptr url_ptr;
		policy_api::policy_work_ptr policy_ptr;
		std::vector<std::string> in, out;
		if ( false == map_ptr->get_one_url(url_no,url_ptr) )
			return ;

		if ( nullptr == policy_api::get_server() || nullptr == (policy_ptr=policy_api::get_server()->clone_policy(policy_name)) )
		{
			map_ptr->finished_one_url(url_no);
			return ;
		};
		//Noise_log("[%s](%d) Policy %s | %s\n", __FUNCTION__, __LINE__, policy_ptr->name() , url_ptr->url_origin.c_str() );

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
		policy_api::get_server()->free_policy(policy_ptr);
	}
























};