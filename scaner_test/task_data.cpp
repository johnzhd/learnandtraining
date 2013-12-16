#include "stdafx.h"
#include "task_data.h"


#include <http_tools.hpp>

#include <boost/date_time/local_time/local_time.hpp> 


namespace task_data
{
	/////////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////////////////////
	task_start_struct::task_start_struct(std::string url, std::string head, std::string trigger)
		:main_url(url),base_head(head),base_trigger(trigger),running_trigger(""),running_cookie(""),running_head(""),update_mutex()
	{
	};

	task_start_struct::~task_start_struct()
	{
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
		str += system_log::format( "_%04d_%02d_%02d", curr->tm_year, curr->tm_mon, curr->tm_mday );

		
		return str;  


	};

	std::string task_start_struct::format_url(std::string url)
	{
		std::string str[5];
		if ( false == http_tools::get_url_info(url,&str[0],&str[1],&str[2],&str[3],&str[4]) )
			return "";

		return str[0]+"://"+str[1]+ ":" + str[2]+"/"+str[3]+"?"+str[4];
	};

	std::string task_start_struct::format_head(std::string head)
	{
		size_t npos;
		npos = head.find_first_not_of("\r\n \t");
		head.erase(0,npos);
		npos = head.find_last_not_of("\r\n \t");
		if( npos != std::string::npos )
			head.erase(npos);

		if ( head.empty())
			return "";

		head += "\r\n";
		return head;
	};

	std::string task_start_struct::format_trigger(std::string trigger)
	{
		size_t npos;
		npos = trigger.find_first_not_of("\r\n \t");
		trigger.erase(0,npos);
		npos = trigger.find_last_not_of("\r\n \t|");
		if( npos != std::string::npos )
			trigger.erase(npos);

		if ( trigger.empty() )
			return "";

		trigger += "||";
		return trigger;
	};


};




namespace task_data
{

task_url_struct::task_url_struct(std::string url)
	:url_origin(url),b_post(false),first_send(),first_recv(),translate_body(L""),running_trigger("")
{
	running_thread = -2;
};

task_url_struct::~task_url_struct()
{
};



};


namespace task_data
{

task_err_page::task_err_page()
	:recv_data(L""),failed_type(Basic_404)
{
};

task_err_page::~task_err_page()
{
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
}

task_result_struct::~task_result_struct()
{
};



};



namespace task_data
{

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
};

task_map::~task_map()
{
};

std::string task_map::get_task_union_id()
{
	return task_union_id;
};

bool task_map::insert_new_url( std::string url )
{
	assert(!url.empty());
	if ( url.empty() )
		return false;
	boost::mutex::scoped_lock lock(url_mutex);
	auto it = url_no_map.find(url);
	if ( it != url_no_map.end() )
		return true;

	size_t no = next_no++; // maybe use other algorithm

	try
	{
	url_no_map.insert(std::pair<std::string,size_t>(url,no));
	no_url_map.insert(std::pair<size_t,std::string>(no,url));
	url_list.insert(std::pair<size_t,task_url_struct_ptr>(no, task_url_struct_ptr(new task_url_struct(url))));
	}
	catch (...)
	{
		High_log("%s %s memory err.",__FILE__,__LINE__);
		return false;
	};

	return true;


}

bool task_map::insert_new_url( task_url_struct_ptr url )
{
	assert(url != nullptr);
	if ( url == nullptr )
		return false;
	boost::mutex::scoped_lock lock(url_mutex);
	auto it = url_no_map.find(url->url_origin);
	if ( it != url_no_map.end() )
		return true;

	size_t no = next_no++; // maybe use other algorithm
	try
	{
	url_no_map.insert(std::pair<std::string,size_t>(url->url_origin,no));
	no_url_map.insert(std::pair<size_t,std::string>(no,url->url_origin));
	url_list.insert(std::pair<size_t,task_url_struct_ptr>(no, url));
	}
	catch (...)
	{
		High_log("%s %s memory err.",__FILE__,__LINE__);
		return false;
	};
	return true;
};

bool task_map::insert_new_url( std::vector<std::string> url_list )
{
	if ( url_list.empty() )
		return false;

	bool b = false;
	for( auto& url : url_list )
	{
		b = insert_new_url(url) || b; //Be careful!
	};
	return true;
};

bool task_map::fresh_one_url( size_t&url_no, task_url_struct_ptr& url_ptr )
{
	url_no = 0;
	url_ptr = nullptr;
	if ( next_no <= next_work_no )
		return false;

	boost::mutex::scoped_lock lock(url_mutex);

	while ( next_work_no<next_no )
	{
		url_no = next_work_no++;
		if ( get_one_url( url_no,url_ptr ) && url_ptr->running_thread == -2 )
		{
			return true;
		}
	};

	return false;
};

void task_map::finished_one_url( task_url_struct_ptr url_ptr )
{
	if ( --url_ptr->running_thread <= 0 )
	{
		url_ptr->running_thread = -1;
	}
};

void task_map::finished_one_url( size_t url_no )
{
	boost::mutex::scoped_lock lock(url_mutex);
	auto it = url_list.find(url_no);
	if ( it == url_list.end() || it->second == nullptr)
		return ;

	finished_one_url(it->second);
}

bool task_map::is_url_empty()
{
	if ( next_work_no >= next_no )
		return true;

	return false;
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
	boost::mutex::scoped_lock lock(result_mutex);
	auto it = result_list.find(url_no);
	if ( it == result_list.end() )
	{
		result_list.insert(std::pair<size_t, std::vector<task_result_struct_ptr>>(url_no,std::vector<task_result_struct_ptr>()));
		it = result_list.find(url_no);
		if ( it == result_list.end() )
			return false;
	}

	it->second.push_back(result_ptr);
	
	return true;
};





};


namespace task_data
{

task_map_list::task_map_list()
	:m_list(),op_mutex()
{
};

task_map_list::~task_map_list()
{
};

bool task_map_list::insert_new(task_map_ptr ptr )
{
	if ( ptr == nullptr || ptr->start_ptr == nullptr || ptr->get_task_union_id().empty() )
		return false;

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