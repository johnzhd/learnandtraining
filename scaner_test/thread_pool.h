#pragma once

#include <vector>


#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition.hpp>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>


#include <atomic> 

#include "thread_work_queue.h"



class thread_pool : public boost::enable_shared_from_this<thread_pool>,
	private boost::noncopyable
{
public:
	thread_pool(void);
	~thread_pool(void);
protected:
	std::atomic_size_t total_count;
	std::atomic_size_t running_count;
protected:
	//base
	void clear()
	{
		total_count = 0;
		running_count = 0;
		work_queue_content.clear();
		work_queue_thread.clear();
	};
	//data
	thread_work_queue work_queue_content;
	//thread
	typedef boost::shared_ptr<boost::thread> thread_ptr;
	//boost::thread_group 
	std::vector<thread_ptr> work_queue_thread; // save thread ptr to interrupt, when stop()
	boost::mutex thread_queue_mutex; // for queue safe
#define LOCKER_T boost::mutex::scoped_lock locker(thread_queue_mutex)


	boost::mutex thread_action_mutex; // for action
	boost::condition thread_wait_condition; // wait action signel



public:
	bool start( size_t total );
	bool stop();
	bool is_running();
	size_t free();
public:
	void thread_run(); // working threads

	// data operation
public:
	bool push( function_type func );
protected:
	bool pull( function_type& func );
};

namespace thread_pool_api
{
	typedef boost::shared_ptr<thread_pool> thread_pool_ptr;
	thread_pool_ptr get_pool(); //create one NEW
}
