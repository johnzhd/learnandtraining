#pragma once

#include <string>
#include <sstream>
#include <boost/format.hpp>

#include "file_base_tools.h"

namespace system_log
{
	enum log_type
	{
		Silence = -1,
		High_Err = 0,
		Warning,
		Function,
		Noise,		
		Debug
	};

	void set_log_show_type(log_type ty);

	void log( std::string str, log_type ty = Function );

	std::string format( const char* boost_format );
	
	template<typename T1>
	std::string format( const char* boost_format, T1 t1 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}

	template<typename T1, typename T2>
	std::string format( const char* boost_format, T1 t1, T2 t2 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		fmter % t2;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}

	template<typename T1, typename T2, typename T3>
	std::string format( const char* boost_format, T1 t1, T2 t2, T3 t3 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		fmter % t2;
		fmter % t3;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}

	template<typename T1, typename T2, typename T3, typename T4>
	std::string format( const char* boost_format, T1 t1, T2 t2, T3 t3, T4 t4 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		fmter % t2;
		fmter % t3;
		fmter % t4;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	std::string format( const char* boost_format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		fmter % t2;
		fmter % t3;
		fmter % t4;
		fmter % t5;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	std::string format( const char* boost_format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6 )
	{
		boost::format fmter(boost_format);
		fmter % t1;
		fmter % t2;
		fmter % t3;
		fmter % t4;
		fmter % t5;
		fmter % t6;
		std::ostringstream stringStream;
		stringStream << fmter;
		return  stringStream.str();
	}
	
#define Noise_log(p_format, ...) system_log::log(system_log::format(p_format, ##__VA_ARGS__), system_log::Noise)
#define Function_log(p_format, ...) system_log::log(system_log::format(p_format, ##__VA_ARGS__), system_log::Function)
#define Warning_log(p_format, ...) system_log::log(system_log::format(p_format, ##__VA_ARGS__), system_log::Warning)
#define High_log(p_format, ...) system_log::log(system_log::format(p_format, ##__VA_ARGS__), system_log::High_Err)

#define Debug_log(p_format, ...) system_log::log(system_log::format(p_format, ##__VA_ARGS__), system_log::Debug)
}

namespace system_log
{
#define TASK_STR_SUCCESS	("success.")
#define TASK_STR_FAILED		("failed.")

#define TASK_STR_POLICY_INIT_ERR	("(C0000001) init policy err.")
#define TASK_STR_THREAD_POOL_INIT_ERR	("(C0000002) init pool err.")

#define TASK_STR_MEMORY_PTR_ERR		("(C0010001) Out of memory.")


#define TASK_STR_SAME_TASK_ERR	("(C0020001) exist same task in queue.")
#define TASK_STR_URL_ERR		("(C0020002) input url is not acceptable.")
#define TASK_STR_PROTOCAL_ERR		("(C0020003) not support protocal.")

	inline size_t system_err( size_t n ){return 0xFFFFFFFF & (0x80010000|(n&0xFFFF));};
	inline size_t function_err( size_t n ){return 0xFFFFFFFF & (0x80020000|(n&0xFFFF));};
	inline size_t data_err( size_t n ){return 0xFFFFFFFF & (0x80030000|(n&0xFFFF));};

	inline size_t op_err( size_t n ){return 0xFFFFFFFF & (0x40010000|(n&0xFFFF));};

	inline std::string err_ret(size_t n)
	{
		std::stringstream ss;
		switch ( 0xFFFF0000 & n )
		{
		case 0x80010000:
			ss << "System err ";
			break;
		case 0x80020000:
			ss << "Function err ";
			break;
		case 0x80030000:
			ss << "Data err ";
			break;
		case 0x40010000:
			ss << "Operator refused ";
		default:
			ss << "Err ";
			break;
		};

		ss << std::hex <<  n << ".";

		return ss.str();
	};

	inline std::string success_ret(){
		return TASK_STR_SUCCESS;
	};

};

