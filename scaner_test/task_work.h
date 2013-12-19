#pragma once

#include <set>
#include <string>
#include <vector>
#include <atomic> 

#include "task_data.h"
#include "policy_work.h"

namespace task_work
{
	std::string start( std::string url, std::string head, std::string trigger );


	void pull_and_download_url(task_data::task_map_ptr map_ptr);


	inline size_t crawl_page( std::vector<wchar_t>& body, std::set<std::string>* url_list, std::string page_url );
	inline size_t crawl_page( const std::wstring& body, std::set<std::string>* url_list, std::string page_url );
	size_t crawl_page( const std::string& body, std::set<std::string>* url_list, const std::string& page_url );


	void run_policy(task_data::task_map_ptr map_ptr, size_t url_no, policy_api::policy_work_ptr policy_ptr);
	void run_policy_name(task_data::task_map_ptr map_ptr, size_t url_no, std::string policy_ptr);

};

/*
thread_pool_api::thread_pool_ptr thread_pool_ptr_point;
policy_api::policy_server_ptr policy_server_ptr_point;
scan_task::task_map_ptr task_map_ptr_point;

template<typename Connection_Ptr>
void on_recv(Connection_Ptr socket_ptr_, const boost::system::error_code& e, size_t bytes_transferred_);

template<typename Connection_Ptr>
void on_send(Connection_Ptr socket_ptr_, const boost::system::error_code& e, size_t bytes_transferred_);
*/

