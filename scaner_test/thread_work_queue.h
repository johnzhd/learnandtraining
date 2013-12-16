#pragma once

#include <deque>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


typedef boost::function<void ()> function_type;

class thread_work_queue
{
public:
	thread_work_queue(void)	{clear();};
	~thread_work_queue(void){clear();};
protected:
	std::deque<function_type> deque_content;
	boost::mutex content_mutex;
#define LOCKER boost::mutex::scoped_lock locker(content_mutex)
public:
	bool pushback( function_type& in )
	{
		LOCKER;
		deque_content.push_back(in);
		return true;
	};
	bool pushfront( function_type& in )
	{
		LOCKER;
		deque_content.push_front(in);
		return true;
	};
	bool pullback( function_type& out )
	{
		LOCKER;
		if ( empty() )
			return false;
		out = deque_content.back();
		deque_content.pop_back();
		return true;
	};
	bool pullfront( function_type& out )
	{
		LOCKER;		
		if ( empty() )
			return false;
		out = deque_content.front();
		deque_content.pop_front();
		return true;
	};
	bool empty(){return deque_content.empty();};
	void clear(){LOCKER; deque_content.clear();};
	size_t size(){return deque_content.size();};
};

