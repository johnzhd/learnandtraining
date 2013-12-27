#include "stdafx.h"
#include "task_http.h"


#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition.hpp>


#include "system_log.h"

#include "html_parser_api.h"

#include "http_tools.hpp"

#include "iocp_server.hpp"



std::string task_http::control_download( IN task_data::task_start_struct_ptr base_ptr, IN task_data::task_url_struct_ptr url_ptr,
		OUT std::string& cookie, OUT std::vector<task_data::net_pack_ptr>& net_stream)

{
	// first net io

	// send_raw = func( base_ptr, url_ptr ) / 301 :  func( last raw ) / 100 : null

	// ip port protocal = func ( url_ptr )

	// connect ip port with protocal

	// send 

	// recv head : switch ( policy_func ( state ) )
	//               100 ==> continue;
	//               301 ==> recv ==> running_cookie / 301 count / update url_work
	//                   ==> func( 301 count, url_work, base_ptr->running_trigger ) => continue / return 
	//               400 ==> info => return
	//               500 ==> err => return
	//               200 ==> function( content-type, url_ptr->running_trigger )
	//                     ==> page recv

	//

	std::string url_work,protocal,domain,port;
	bool b_new_connect = true;
	int ttl_301 = 3;
	std::string str_ret = TASK_STR_FAILED;

	url_work = url_ptr->url_origin;
	iocp_server_api::iocp_server_ptr server_ptr = nullptr;
	server_ptr = iocp_server_api::get_server();
	if ( server_ptr == nullptr )
		return false;

	connection_ptr http_ptr = nullptr;
#ifdef USE_SSL
	connection_ssl https_ptr = nullptr;
#endif


	boost::mutex net_single_mutex;
	boost::mutex::scoped_lock locker(net_single_mutex);
	boost::condition thread_wait_condition;
	


	task_data::net_pack_ptr work_data_ptr;
	std::string last_state;
	http_tools::send_package_type http_action;
	std::string running_ref;
	std::string running_cookie;
	bool b_ssl =false;

	http_action = url_ptr->b_post;

	size_t sum,step;
	boost::system::error_code e_io;

	do
	{
		work_data_ptr.reset(new task_data::net_pack());

		if ( b_new_connect )
		{
			if ( false == http_tools::get_url_info(url_work,&protocal,&domain,&port,nullptr,nullptr) )
			{
				assert(false);
				str_ret = "err url infomation";
				Noise_log("[%s] %s. %s \n", __FUNCTION__,str_ret, url_work );
				break;
			};

			auto func_connect = [&](connection_ptr ptr, const boost::system::error_code& e)
			{
				if ( e )
				{
					Debug_log( "%s (%s): %d %s\n", __FILE__,__LINE__,e.value(), e.message() );
					http_ptr->clear();
					http_ptr = nullptr;
					e_io = e;
					return ;
				}
				thread_wait_condition.notify_all();
			};

#ifdef USE_SSL
			b_ssl = http_tools::is_ssl(protocal);
			if ( b_ssl )
			{
				http_ptr = server_ptr->create_connect(domain,port,func_connect);
				thread_wait_condition.wait(locker);
				if ( http_ptr == nullptr )
				{
					Noise_log("[%s] Net connect failed. %s \n", __FUNCTION__, url_work );
					break;
				};
			}
			else
#endif
			{
				

				http_ptr = server_ptr->create_connect(domain,port,func_connect);
				thread_wait_condition.wait(locker);
				if ( e_io )
				{
					str_ret = system_log::format("(%d) %s", e_io.value(), e_io.message());
					Noise_log("[%s] %s. %s \n", __FUNCTION__,str_ret, url_work );
					break;
				};
			}
		}; // if ( b_new_connect )

		str_ret == make(work_data_ptr->origin_stream_request, last_state, running_ref, running_cookie, http_action );
		if ( str_ret != TASK_STR_SUCCESS)
		{
			// memory
			Noise_log("[%s] %s. %s \n", __FUNCTION__,str_ret, url_work );
			break;
		};


		auto func_io = [&](connection_ptr ptr, const boost::system::error_code& e, size_t bytes_transferred_)
		{
			if ( e )
			{
				Debug_log( "%s (%s): %d %s\n", __FILE__,__LINE__,e.value(), e.message().c_str() );
				http_ptr->clear();
				http_ptr = nullptr;
				e_io = e;
				return ;
			}
			thread_wait_condition.notify_all();

			step = bytes_transferred_;
		};

		// send 
		sum = 0;
		while ( sum < work_data_ptr->origin_stream_request.size() )
		{
#ifdef USE_SSL
			if ( b_ssl )
			{
			server_ptr->send( https_ptr, &*(work_data_ptr->origin_stream_request.begin() + sum ),
				work_data_ptr->origin_stream_request.size() - sum, func_io);
			}
			else
#endif
			{
			server_ptr->send( http_ptr, &*(work_data_ptr->origin_stream_request.begin() + sum ),
				work_data_ptr->origin_stream_request.size() - sum, func_io);
			}

			thread_wait_condition.wait(locker);
			if ( e_io )
			{
				break;
			};
			sum += step;
		}

		if ( e_io )
		{
			str_ret = system_log::format("(%d) %s", e_io.value(), e_io.message());
			Noise_log("[%s] %s. %s \n", __FUNCTION__,str_ret, url_work );
			break;
		};

		
		// recv
		for (; !e_io ;)
		{
#ifdef USE_SSL
			if ( b_ssl )
			{
			server_ptr->recv( https_ptr,2048, func_io);
			}
			else
#endif
			{
			server_ptr->recv( http_ptr,2048, func_io);
			}

			thread_wait_condition.wait(locker);
			if ( e_io )
			{
				break;
			};

			parser_io_net( work_data_ptr, http_ptr->buff_temp(), step );
		}

	}
	while ( ttl_301-- > 0 );

	return str_ret;
};

bool control_parser( IN task_data::task_map_ptr base_ptr, IN task_data::task_url_struct_ptr url_ptr )
{
	// set
	// url_ptr->running_trigger = func (base_ptr->start_ptr->running_trigger, url_ptr->get_last_page()->type )
	
	// check
	// base_ptr->start_ptr->running_trigger
	// url_ptr->running_trigger
	// url_ptr->get_last_page()->type
	// url_ptr->ttl

	// parser
	// url_ptr->get_last_page()->origin_stream_response
	//          ==> url ==> filter (domain ) ==> urls list

	// prepair
	// urls list ==> filter ==> urls list
	// urls list =  build ( request ( ref , cookie, http action, params ), urls list )
	
	// insert 
	// in urls list
	//    ==> base_ptr->insert_new_url_params( ref, cookie, url )
	//


};