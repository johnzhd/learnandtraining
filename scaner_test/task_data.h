#pragma once

#include <atomic>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <string>

#include <sstream>


#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

////////////////////////////////////////////////////////////////
// log message



namespace task_data
{
	



class task_start_struct
{
public:
	task_start_struct(std::string url, std::string head, std::string trigger);
	~task_start_struct();
	void clear();
public:
	std::string main_url;
	std::string base_head;
	std::string base_trigger;
public:
	//******************************
	//* trigger meaning list:
	//*
	//******************************

	std::string running_trigger;
	std::string running_cookie;
	std::string running_head;
public:
	boost::mutex update_mutex;
	bool update_running_cookie( std::string cookie )
	{
		boost::mutex::scoped_lock locker(update_mutex);
		running_cookie = cookie;
		return true;
	};
	bool get_running_cookie( std::string& cookie )
	{
		boost::mutex::scoped_lock locker(update_mutex);
		cookie = running_cookie;
		return true;
	};

public:
	static std::string make_union_id( std::string url );

};




// mini task struct
class task_url_struct
{
public:
	task_url_struct(std::string url);
	~task_url_struct();
public:
	std::string url_origin;
	bool b_post;
	std::vector<unsigned char> first_send;
	std::vector<unsigned char> first_recv;

	std::wstring translate_body;

	// copy from task struct
	// main url don't remove base mark
	// other urls remove base mark; add page mark
	std::string running_trigger;
	
	// -2 is new
	// -1 is be fresh out
	// 0 is finished
	// n is running n threads
	std::atomic_int running_thread;
};

//err page
enum err_page_type
{
	Basic_404 = 0,
	Self_200,
	Main_Page
};
class task_err_page
{
public:
	task_err_page();
	~task_err_page();
public:
	std::wstring recv_data;
	err_page_type failed_type;
public:
	void set_err_page( err_page_type type, std::wstring data = L"" );
};


//result
class task_result_struct
{
public:
	task_result_struct(std::string url, std::string id, std::string message, std::vector<wchar_t>& data, bool copy = true);
	~task_result_struct();
public:
	std::string vul_url;
	std::string vul_id;
	std::string vul_message;
	std::vector<wchar_t> url_data;
};

typedef boost::shared_ptr<task_start_struct> task_start_struct_ptr;
typedef boost::shared_ptr<task_url_struct> task_url_struct_ptr;
typedef boost::shared_ptr<task_result_struct> task_result_struct_ptr;
typedef boost::shared_ptr<task_err_page> task_err_page_ptr;


class task_map
{
public:
	task_map(task_start_struct_ptr ptr);
	~task_map();
protected:
	std::string task_union_id;
	
	std::atomic_size_t next_no;

	std::atomic_size_t next_work_no;

	std::atomic_size_t finished_no;

public:
	task_start_struct_ptr start_ptr;
	task_err_page_ptr err_page_ptr;

	std::map<std::string, size_t> url_no_map;  // only insert&clear not erase
	std::map<std::size_t,std::string> no_url_map;// op with url_no_map together

	std::map<size_t, task_url_struct_ptr> url_list;
	std::map<size_t, std::vector<task_result_struct_ptr>> result_list;

	boost::mutex url_mutex;
	boost::mutex result_mutex;


public:
	std::string get_task_union_id();



	bool insert_new_url( std::string url );
	bool insert_new_url( task_url_struct_ptr url );
	bool insert_new_url( std::set<std::string>& url_list );

	bool fresh_one_url( size_t&url_no, task_url_struct_ptr& url_ptr );
	void finished_one_url( task_url_struct_ptr url_ptr );
	void finished_one_url( size_t url_no );

	bool is_url_empty(); // is url_list content nothing new;
	int url_list_remain();

	bool get_one_url( size_t url_no, task_url_struct_ptr& url_ptr );

	bool insert_result( size_t url_no, task_result_struct_ptr result_ptr );

	void test_task_finished_quit();
};

typedef boost::shared_ptr<task_map> task_map_ptr;

class task_map_list
{
public:
	task_map_list();
	~task_map_list();
public:
	std::list<task_map_ptr> m_list;
	boost::mutex op_mutex;
public:
	bool insert_new(task_map_ptr ptr );
	void finished_one(std::string task_union_id); 
};

typedef boost::shared_ptr<task_map_list> task_map_list_ptr;

task_map_list_ptr get_globle_task_map();

};



