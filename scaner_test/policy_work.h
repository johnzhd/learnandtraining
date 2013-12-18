

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

class trigger_base
{
public:
	trigger_base(std::string trigger);
	~trigger_base();
public:
	bool hit_trigger( std::string trigger );
protected:
	std::vector<std::string> v_trigger;
public:
	static bool divil_trigger( std::string trigger, std::vector<std::string>& v );
	static std::string format_trigger(std::vector<std::string>& v_t );
	static std::string format_trigger(std::string trigger);
	static std::string format_trigger(std::string trigger, std::string add);
};
	
class policy_base 
	 : public boost::enable_shared_from_this<policy_base>,
	 private boost::noncopyable
{
public:
	policy_base();
	~policy_base();
public:
	lua_State*& point();
	bool load(const char * filePathName);
	bool empty() const;
	void clear();
protected:
	lua_State *lu_ptr;
public:
	std::string m_version;
	std::string m_vul_union_id;
	std::string m_triggle;

public:
	enum init_environment_type
	{
		INIT_WHOLE = 0,
		INIT_WORK,
	};
	bool init_environment(init_environment_type type = INIT_WORK);

	bool call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in );
protected:
	bool get_globle_value();
	inline std::string get_string_globle(const char* name);
	inline int get_int_globle(const char* name);
	inline bool set_function_globle( const char* name, lua_CFunction func);

};


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
	// static policy api
	// scaner main charge
	// policy Provided infomation

public:
	bool call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in );
	// action policy api
	// policy main charge
	// scaner provided functions
	std::string get_vul_id();
public:

	//base
protected:
	lua_State *lu_ptr;
	std::string str_name;
};


typedef boost::shared_ptr<policy_base> policy_base_ptr;
typedef boost::shared_ptr<policy_work> policy_work_ptr;


class policy_base_server
	: public boost::enable_shared_from_this<policy_base_server>,
	 private boost::noncopyable
{
public:
	policy_base_server();
	~policy_base_server();
protected:
	policy_base_ptr origin_base;
	std::map<std::string, policy_base_ptr> base_map;
public:
	void insert_plugins_name( std::string name );
public:
	bool init();
	void clear();
public:
	void free_policy( policy_work_ptr ptr );
	policy_work_ptr get_policy( std::string name ) const;
	policy_work_ptr get_policy( size_t no ) const;
	size_t get_policy_count() const;

	bool get_policys( std::string trigger, std::vector<policy_work_ptr>& policy_list) const;
public:
	policy_work_ptr create_copy(policy_base_ptr base,std::string name) const;
	void delete_copy(policy_work& sub) const;
};


typedef boost::shared_ptr<policy_base_server> policy_server_ptr;
policy_server_ptr get_server();


};
