#pragma once



extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


#include <string>
#include <vector>
#include <map>



#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


#define POLICY_STR_GET_APP_PATH	file_base_tools::get_app_path()

#define POLICY_STR_MAIN_FILE ("main.lua")
#define POLICY_STR_VERSION	("GLOBLE_VERSION")
#define POLICY_STR_UNION_ID	("GLOBLE_UNIONID")
#define POLICY_STR_TRIGGLE	("GLOBLE_TRIGGER_TYPE")


#define GLOBLE_FUNC_NAME_INSERT_LOAD	("InsertNewPlugins")

#define POLICY_STR_FUNCTION_FIRST_CALL	("Init_Lua")
#define POLICY_INT_DEFAULT_IN_COUNT		(3)
#define POLICY_INT_DEFAULT_OUT_COUNT	(3)


namespace policy_api
{
	enum init_environment_type
	{
		INIT_WHOLE = 0,
		INIT_WORK,
	};

	

};

namespace policy_api{


//****************************************************
// first : base load file into buff
//         base put buff into lua
//         base read lua info
//         base close lua
// second : base put buff into lua
//          base init lua environment
//          base deliver lua to work
//****************************************************
class policy_base 
	 : public boost::enable_shared_from_this<policy_base>,
	 private boost::noncopyable
{
public:
	policy_base(init_environment_type type);
	~policy_base();
public:
	inline char* buff();
	inline size_t buff_size();
	std::string name();
	bool load(const char * filePathName);
	bool empty() const;
	void clear();
protected:
	boost::mutex lua_mutex;
public:
	std::string m_version;
	std::string m_vul_union_id;
	std::string m_triggle;
	std::string m_name;
	init_environment_type m_type;

	std::vector<char> m_buff;
public:
	
	lua_State * clone_one();
	inline void close_one(lua_State * lua);
protected:
	bool init_environment(lua_State * ls_lua, init_environment_type type = INIT_WORK);
	
	inline int get_int_globle(lua_State * ls_lua, const char* name);
	inline std::string get_string_globle(lua_State * ls_lua, const char* name);

	inline bool set_function_globle(lua_State * ls_lua,  const char* name, lua_CFunction func);

};


};


namespace policy_api
{


class policy_work
{
public:
	policy_work(lua_State *lu, std::string name);
	~policy_work(void);
public:
	lua_State*& point();
	std::string name();
	bool empty() const;
	void clear();
public:
#ifdef _DEBUG
	bool run_demo();
#endif

public:
	bool call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in );
public:
	
	//base
protected:
	lua_State *lu_ptr;
	std::string str_name;
	boost::mutex lua_mutex;
protected:
	
};


};