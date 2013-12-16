#include "stdafx.h"
#include "thread_pool.h"

#include <boost/bind.hpp>


namespace thread_pool_api
{
	thread_pool_ptr get_pool()
	{
		return thread_pool_ptr( new thread_pool() );
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
		SET_WAIT(locker);
		function_type f;

		while ( false == pull(f) )
			thread_wait_condition.wait( locker );
		running_count++;
		f();
		running_count--;
	}
};

bool thread_pool::push( function_type func )
{
	if ( false == is_running() )
		return false;
	work_queue_content.pushback(func);
	thread_wait_condition.notify_one();
	return true;
}

bool thread_pool::pull( function_type& func )
{
	bool b = work_queue_content.pullfront( func );
	if ( !work_queue_content.empty() )
		thread_wait_condition.notify_one();

	return b;
}

