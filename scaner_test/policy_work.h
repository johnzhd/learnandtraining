

#pragma once


extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <string>
#include <vector>
#include <map>



#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include "policy_base.h"


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

};

namespace policy_api
{

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
	size_t get_policy_count(std::string trigger = "") const;
	bool get_policys( std::string trigger, std::vector<std::string>& policy_list) const;



	policy_work_ptr clone_policy( std::string name ) const;
	policy_work_ptr clone_policy( size_t no ) const;

	
	void free_policy( policy_work_ptr ptr );

};


typedef boost::shared_ptr<policy_base_server> policy_server_ptr;
policy_server_ptr get_server();


};
