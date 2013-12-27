#include "stdafx.h"
#include "task_data.h"


#include <http_tools.hpp>

#include <boost/date_time/local_time/local_time.hpp> 

#include "http_tools.hpp"

#include "policy_work.h"


namespace task_data
{
	/////////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////////////////////
	task_start_struct::task_start_struct(std::string url, std::string head, std::string trigger)
		:main_url(url),base_head(head),base_trigger(trigger),running_trigger(""),running_cookie(""),running_head(""),update_mutex(),
		domain_sub_level_min(2),ttl_max(65535)
	{
		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};

	task_start_struct::~task_start_struct()
	{
		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};

	void task_start_struct::clear()
	{
		boost::mutex::scoped_lock locker(update_mutex);
		main_url.clear();
		base_head.clear();
		base_trigger.clear();
		running_trigger.clear();
		running_cookie.clear();
		running_head.clear();
		ttl_max = 65535;
		domain_sub_level_min = 2;
	};

	std::string task_start_struct::make_union_id( std::string url )
	{
		// create some number
		// to string
		std::string str;
		str = http_tools::get_domain(url);

		std::time_t t;
		std::tm t_local;
		std::time(&t);
		std::tm* curr = boost::date_time::c_time::localtime(&t, &t_local);
		str += system_log::format( "_%04d_%02d_%02d", curr->tm_year+1900, curr->tm_mon, curr->tm_mday );

		
		return str;  


	};

	bool task_start_struct::update_running_cookie( std::string cookie )
	{
		boost::mutex::scoped_lock locker(update_mutex);
		running_cookie = cookie;
		return true;
	};
	bool task_start_struct::get_running_cookie( std::string& cookie )
	{
		boost::mutex::scoped_lock locker(update_mutex);
		cookie = running_cookie;
		return true;
	};

	void task_start_struct::format_start()
	{
		boost::mutex::scoped_lock locker(update_mutex);

		main_url = http_tools::format_url(main_url);
		base_head = http_tools::format_head(base_head);
		base_trigger = policy_api::trigger_base::format_trigger(base_trigger);

		running_trigger = base_trigger;
		running_head = base_head;
		http_tools::get_new_cookie( running_head, "", running_cookie);

		if ( domain_sub_level_min == 0 || domain_sub_level_min > 255 )
			domain_sub_level_min = 2;

	};

	


};




namespace task_data
{

task_url_struct::task_url_struct(std::string url)
	:url_origin(url),b_post(false),first_send(),first_recv(),translate_body(L""),running_trigger("")
{
	running_thread = -2;

	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

task_url_struct::~task_url_struct()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};



};


namespace task_data
{

task_err_page::task_err_page()
	:recv_data(L""),failed_type(Basic_404)
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

task_err_page::~task_err_page()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

void task_err_page::set_err_page( err_page_type type, std::wstring data )
{
	failed_type = type;
	recv_data = data;
};


};


namespace task_data
{

task_result_struct::task_result_struct(std::string url, std::string id, std::string message, std::vector<wchar_t>& data, bool copy )
	:vul_url(url),vul_id(id),vul_message(message)
{
	url_data.clear();
	if ( copy )
	{
		url_data.resize( data.size() );
		std::copy(data.begin(),data.end(),url_data.begin());
	}
	else
	{
		url_data.swap(data);
	}

	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
}

task_result_struct::~task_result_struct()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};



};



namespace task_data
{
const size_t task_map::running_url_max = 30;

task_map::task_map(task_start_struct_ptr ptr)
	:start_ptr(ptr),err_page_ptr(nullptr),url_no_map(),no_url_map(),url_list(),result_list(),
	url_mutex(),result_mutex()
{
	assert(start_ptr != nullptr);
	assert(!start_ptr->main_url.empty());
	err_page_ptr.reset(new task_err_page());

	if ( start_ptr == nullptr )
	{
	task_union_id = "";
	}
	else
	{
		task_union_id = start_ptr->make_union_id(start_ptr->main_url);
	}

	next_no = 0;

	next_work_no = 0;

	finished_no = 0;

	running_url_count = 0;

	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

task_map::~task_map()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

std::string task_map::get_task_union_id()
{
	return task_union_id;
};

bool task_map::insert_new_url( std::string url )
{
	return insert_new_url( task_url_struct_ptr(new task_url_struct(url)) );
}

bool task_map::insert_new_url( task_url_struct_ptr url )
{
	assert(url != nullptr);
	if ( url == nullptr )
		return false;
	
	http_tools::format_url_params(url->url_origin, url->params_data);
	boost::mutex::scoped_lock lock(url_mutex);


	size_t no = next_no++; // maybe use other algorithm
	try
	{
		std::pair<std::map<std::string,size_t>::iterator,bool> ret;
		ret = url_no_map.insert(std::pair<std::string,size_t>(url->url_origin,no));
		if ( ret.second == false ) // already exist
			return true;

		assert(no == ret.first->second);

		no_url_map.insert(std::pair<size_t,std::string>(ret.first->second,url->url_origin));
		url_list.insert(std::pair<size_t,task_url_struct_ptr>(ret.first->second, url));
	}
	catch (...)
	{
		url_no_map.erase(url->url_origin);
		no_url_map.erase(no);
		url_list.erase(no);
		High_log("%s %s memory err.",__FILE__,__LINE__);
		return false;
	};
	return true;
};

bool task_map::insert_new_url( std::set<std::string>& url_list )
{
	if ( url_list.empty() )
		return false;

	bool b = false;
	for( auto& url : url_list )
	{
		b = insert_new_url(url) || b; //Be careful!
	};
	size_t no = next_no;
	Noise_log("[%s] next no %d.\n", __FUNCTION__, no );
	return b;
};

void task_map::insert_new_url_params( std::string page, std::map<std::string,std::vector<unsigned char>>& params,
	std::string url_ref, bool post )
{
	assert(page.empty() != true);
	if ( page.empty() )
		return ;

	http_tools::format_url_params(page, params);

	size_t no = next_no++; // maybe use other algorithm
	{
		boost::mutex::scoped_lock lock(url_mutex);
		std::pair<std::map<std::string,size_t>::iterator,bool> ret;
		ret = url_no_map.insert(std::pair<std::string,size_t>(page,no));
		if ( ret.second == false ) // already exist
			return ;

		assert(no == ret.first->second);
		no_url_map.insert(std::pair<size_t,std::string>(ret.first->second,page));


		// prepair for make up send package
		task_url_struct_ptr ptr;
		ptr.reset(new task_url_struct(page) );

		ptr->b_post = post;

		ptr->params_data.swap( params );
		ptr->url_refrence = url_ref;

		url_list.insert(std::pair<size_t,task_url_struct_ptr>(ret.first->second, ptr));
	}
	return ;
};

bool task_map::fresh_one_url( size_t&url_no, task_url_struct_ptr& url_ptr )
{
	url_no = 0;
	url_ptr = nullptr;
	if ( next_no <= next_work_no || running_url_count >= running_url_max )
		return false;

	//boost::mutex::scoped_lock lock(url_mutex);

	while ( next_work_no<next_no )
	{
		url_no = next_work_no++;
		
		if ( get_one_url( url_no,url_ptr ) && url_ptr->running_thread == -2 )
		{
			running_url_count++;
			Noise_log("[%s] work no %d / %d.\n", __FUNCTION__, url_no, next_no.load(std::memory_order_relaxed) );
			return true;
		}
	};

	return false;
};

void task_map::finished_one_url( size_t url_no )
{
	task_url_struct_ptr ptr = nullptr;

	// only lock when search running
	{
		boost::mutex::scoped_lock lock(url_mutex);
		auto it = url_list.find(url_no);
		if ( it == url_list.end() || it->second == nullptr)
			return ;
		ptr = it->second;
	}

	finished_one_url(ptr);
}

void task_map::finished_one_url( task_url_struct_ptr url_ptr )
{
	if ( --url_ptr->running_thread <= 0 )
	{
		url_ptr->running_thread = -1;
	}

	if ( url_ptr->running_thread == -1 )
	{
		running_url_count--;
		Noise_log("[%s] Finished one url %s, running_count %d \n", __FUNCTION__, get_task_union_id(), running_url_count.load(std::memory_order_relaxed) );
		test_task_finished_quit(); 
	};
};

void task_map::test_task_finished_quit()
{
	task_url_struct_ptr ptr;
	size_t i = finished_no;
	for ( ; i<next_no ; i++ )
	{
		if ( false == get_one_url(i,ptr) )
		{
			continue;
		}
		if ( ptr->running_thread != -1 )
		{
			return ;
		};
		if ( i > finished_no )
			finished_no = i;
	};

	task_map_list_ptr manager = get_globle_task_map();
	manager->finished_one(get_task_union_id());
	
};


bool task_map::is_url_empty()
{
	return next_work_no >= next_no;
};

int task_map::url_list_remain()
{
	int n = next_no - next_work_no, n1 = running_url_max-running_url_count;
	return std::min(n , n1 ) ;
};



bool task_map::get_one_url( size_t url_no, task_url_struct_ptr& url_ptr )
{
	boost::mutex::scoped_lock lock(url_mutex);
	auto it = url_list.find(url_no);
	if ( it == url_list.end() || it->second == nullptr)
		return false;

	url_ptr = it->second;
	return true;
};


bool task_map::insert_result( size_t url_no, task_result_struct_ptr result_ptr )
{
	if ( url_no >= next_no || result_ptr == nullptr )
	{
		return false;
	};
	assert( no_url_map.find(url_no) != no_url_map.end() );
	boost::mutex::scoped_lock lock(result_mutex);

	//std::pair<std::map<size_t, std::vector<task_result_struct_ptr>>::iterator,bool> ret;
	//ret = result_list.insert(std::pair<size_t, std::vector<task_result_struct_ptr>>(url_no,std::vector<task_result_struct_ptr>()));
	result_list[url_no].push_back(result_ptr);
	
	return true;
};





};


namespace task_data
{

task_map_list::task_map_list()
	:m_list(),op_mutex()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

task_map_list::~task_map_list()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};

bool task_map_list::insert_new(task_map_ptr ptr )
{
	if ( ptr == nullptr || ptr->start_ptr == nullptr || ptr->get_task_union_id().empty() )
		return false;

	Noise_log("[%s](%d) New one task %s\n", __FUNCTION__, __LINE__, ptr->get_task_union_id() );
	boost::mutex::scoped_lock lock(op_mutex);
	m_list.push_back(ptr);

	return true;
};

void task_map_list::finished_one(std::string task_union_id)
{
	boost::mutex::scoped_lock lock(op_mutex);
	for (auto it = m_list.begin(); it != m_list.end(); it++ )
	{
		if ( (*it)->get_task_union_id() == task_union_id)
		{
			Noise_log("[%s](%d) Finished one task %s\n", __FUNCTION__, __LINE__, task_union_id );
			(*it)->start_ptr = nullptr;
			m_list.erase(it);
			return;
		}
	};
};

};



namespace task_data
{
	task_map_list_ptr g_task_map_list_ptr(nullptr);
	task_map_list_ptr get_globle_task_map()
	{
		if ( g_task_map_list_ptr == nullptr )
		{
			g_task_map_list_ptr.reset(new task_map_list());
		};
		return g_task_map_list_ptr;
	};
};