#include "stdafx.h"
#include "thread_pool.h"

#include <boost/bind.hpp>

#include <thread>


namespace thread_pool_api
{
	thread_pool_ptr g_ptr = nullptr;
	thread_pool_ptr get_pool()
	{
		if ( g_ptr == nullptr )
			g_ptr.reset(new thread_pool());
		return g_ptr;
	}
}


thread_pool::thread_pool(void)
	:thread_queue_mutex(),
	thread_action_mutex(),
	total_count(0),
	running_count(0)
{
	clear();
}


thread_pool::~thread_pool(void)
{
	clear();
}

bool thread_pool::start( size_t total )
{
	stop();
	LOCKER_T;
	total_count = total;
	work_queue_thread.resize(total,nullptr);
	for( size_t i = 0; i < total; i++ )
	{
		work_queue_thread[i].reset( new boost::thread( boost::bind(&thread_pool::thread_run, shared_from_this()) ) );
	};

	return true;
}

bool thread_pool::stop()
{
	LOCKER_T;
	// thread term
	for ( auto it = work_queue_thread.begin(); it != work_queue_thread.end(); it++ )
	{
		(*it)->interrupt();
	}
	// 
	clear();

	return true;
}

size_t thread_pool::free()
{
	int n = total_count - running_count;
	n = n<0 ? 0 : n;
	return static_cast<size_t>(n);
}

bool thread_pool::is_running()
{
	LOCKER_T;
	return work_queue_thread.size()>0;
}

void thread_pool::thread_run()
{
	
	while ( is_running() )
	{
		function_type f;
		boost::mutex::scoped_lock thread_lock_action(thread_action_mutex);

		while ( false == pull(f) )
		{
			thread_wait_condition.wait( thread_lock_action );
			Noise_log("[thread] Awake!! running count %1%, id %2%, queue count %3%.\n", running_count.load(std::memory_order_relaxed), std::this_thread::get_id(), work_queue_content.size());
		};

		thread_wait_condition.notify_one();

		
		running_count++;
		Noise_log("[thread] running count %1%, id %2%, queue count %3%.\n", running_count.load(std::memory_order_relaxed), std::this_thread::get_id(), work_queue_content.size());
		f();
		running_count--;
		
	}
};

bool thread_pool::push( function_type func )
{
	if ( false == is_running() )
		return false;
	work_queue_content.pushback(func);
	thread_wait_condition.notify_all();

	return true;
}

bool thread_pool::pull( function_type& func )
{
	bool b = work_queue_content.pullfront( func );
	if ( !work_queue_content.empty() )
		thread_wait_condition.notify_one();

	return b;
}

