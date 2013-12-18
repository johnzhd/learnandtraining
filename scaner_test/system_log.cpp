#include "stdafx.h"
#include "system_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>

#include <list>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace system_log
{
	class log_static_keeper
	{
	public:
		log_static_keeper()
			:show_type(Function),
			keeper_mutex()
		{
		};
		~log_static_keeper()
		{
		};
	protected:
		log_type show_type;
		std::list<std::string> log_keeper;
	public:
		inline void set_show_type(log_type ty)
		{
			show_type = ty;
		};
		inline bool pass( log_type ty )
		{
			if ( show_type == Debug )
				return show_type == ty;
			return show_type >= ty;
		};
	public:
		inline void insert_keeper( std::string & str )
		{
			boost::mutex::scoped_lock locker(keeper_mutex);
			log_keeper.push_back(str);
		};
		inline void show_text( std::string& str )
		{
			std::cout << str;
		};
		inline void show_err( std::string& str )
		{
			std::cerr << str;
		};
		inline void show_debug( std::string& str )
		{
#ifdef _MSC_VER
			OutputDebugString( str.c_str() );
#else
			show_err( str );
#endif
		};
	public:
		boost::mutex keeper_mutex; // for action
	};



	log_static_keeper g_log_static_keeper;

	void set_log_show_type(log_type ty)
	{
		g_log_static_keeper.set_show_type(ty);
	}

	void log( std::string str, log_type ty )
	{
		switch (ty)
		{
		case system_log::High_Err:
			if ( g_log_static_keeper.pass(ty) )
			{
				g_log_static_keeper.show_err( str );
			}
			g_log_static_keeper.insert_keeper(str);
			break;
		case system_log::Warning:
		case system_log::Function:
		case system_log::Noise:
			if ( g_log_static_keeper.pass(ty) )
			{
				g_log_static_keeper.show_text( str );
				g_log_static_keeper.insert_keeper(str);
			}
			break;
		case system_log::Debug:
			if ( g_log_static_keeper.pass(ty) )
			{
				g_log_static_keeper.show_debug( str );
			}
			break;
		default:
			break;
		}
	}


	std::string format( const char* boost_format )
	{
		return std::string(boost_format);
	}





















}


