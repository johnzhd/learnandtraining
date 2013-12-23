#include "stdafx.h"
#include "policy_work.h"

#pragma comment(lib, "liblua.lib")

//////////////////////////////////////////////////////////////////////////////////////////
// API
//



namespace policy_api
{
	trigger_base::trigger_base(std::string trigger)
	{
		assert(false == trigger.empty());
		if ( false == divil_trigger(trigger,v_trigger) )
		{
			//log
		}

		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};
	trigger_base::~trigger_base()
	{
		Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
	};
	bool trigger_base::hit_trigger(std::string trigger)
	{
		if ( trigger.empty() )
			return false;
		std::vector<std::string> v_t;
		if ( false == divil_trigger(trigger, v_t) )
			return false;

		for ( auto& str : v_trigger)
		{
			// Rule One: Match
			if ( std::find( v_t.begin(), v_t.end(), str ) != v_t.end() )
				return true;
		};
		return false;
	};
	bool trigger_base::divil_trigger(std::string trigger, std::vector<std::string>& v_t )
	{
		if ( trigger.empty() )
			return false;
		size_t nf,nb;
		std::string str_temp;
		for ( nf = 0, nb = trigger.find("||",nf); nf < trigger.length(); nf = nb + 2, nb = trigger.find("||",nf))
		{
			if ( nb == std::string::npos )
			{
				nb = trigger.length();
			};
			str_temp = trigger.substr(nf,nb-nf);
			if ( str_temp.empty() )
				continue;

			std::transform(str_temp.begin(),str_temp.end(),str_temp.begin(),toupper);
			if ( v_t.end() == std::find(v_t.begin(), v_t.end(), str_temp) )
				v_t.push_back(str_temp);
		};
		return false == v_t.empty();
	};

	std::string trigger_base::format_trigger(std::vector<std::string>& v_t )
	{
		if ( v_t.empty() )
			return "";
		std::string str;

		for ( auto s : v_t )
		{
			str += s + "||";
		};
		
		return str;
	};

	std::string trigger_base::format_trigger(std::string trigger)
	{
		std::vector<std::string> v_t;
		if ( false == divil_trigger(trigger,v_t) )
			return "";
		return format_trigger(v_t);
	};

	std::string trigger_base::format_trigger(std::string trigger, std::string add)
	{
		std::string str;
		std::vector<std::string> v_t;
		divil_trigger(trigger,v_t);
		divil_trigger(add,v_t);

		return format_trigger(v_t);
	};




};


//////////////////////////////////////////////////////////////////////////////////////////
// API
//
namespace policy_api
{
	policy_server_ptr g_server = nullptr;

policy_server_ptr get_server()
{
	if ( g_server == nullptr )
		g_server.reset( new policy_base_server() );
	return g_server;
}


policy_base_server::policy_base_server()
	:base_map(),
	origin_base(nullptr)
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};



policy_base_server::~policy_base_server()
{
	Debug_log("%s %08x.\n", __FUNCTION__, reinterpret_cast<size_t>(this));
};


void policy_base_server::insert_plugins_name( std::string name )
{
	base_map.insert( std::pair<std::string,policy_base_ptr>(name,nullptr));
}


bool policy_base_server::init()
{
	//origin_base new
	_ASSERT( origin_base == nullptr );

	try{


		//origin_base init function
		origin_base.reset(new policy_base(INIT_WHOLE) );

		// init_work load ini file
		if ( false == origin_base->load( (POLICY_STR_GET_APP_PATH + POLICY_STR_MAIN_FILE).c_str() ) )
			throw std::string("Load file ") + POLICY_STR_GET_APP_PATH + POLICY_STR_MAIN_FILE + " failed.";

		lua_State * lua_ptr = origin_base->clone_one();
		if ( nullptr == lua_ptr )
			throw std::string("Create policy point failed.");

		policy_work_ptr ptr( new policy_work(lua_ptr, origin_base->name() ));
		if ( nullptr == lua_ptr )
			throw std::string("Create policy op point failed.");

		// init_work run init call
		std::vector<std::string> v_in, v_out;
		v_in.resize(POLICY_INT_DEFAULT_IN_COUNT,"");
		v_out.resize(POLICY_INT_DEFAULT_OUT_COUNT,"");
		if ( false == ptr->call_function(v_out, POLICY_STR_FUNCTION_FIRST_CALL, v_in) )
			throw v_out[0];
		// lua insert file list

		// check file list
		size_t n;
		n = base_map.size();
		std::vector<std::string> eraser_v;
		// loop load plugins
		for( auto it = base_map.begin(); it != base_map.end(); it++ )
		{
			it->second.reset(new policy_base(INIT_WORK));
			if ( it->second == nullptr || false == it->second->load(it->first.c_str()) )
			{
				it->second = nullptr;
				eraser_v.push_back(it->first);
				Function_log( "[%s] load policy failed. %s\n", __FUNCTION__, it->first );
				continue;
			}

		}

		for ( auto& str : eraser_v )
		{
			base_map.erase( str );
		};

		return true;
	}
	catch (std::string& e)
	{
		High_log( "Policy init exception: %s\n", e );
		clear();
	}
	return false;

}

void policy_base_server::clear()
{
	for ( auto it = base_map.begin() ; it != base_map.end() ; it++ )
	{
		if ( it->second )
			it->second->clear();
	}
	base_map.clear();
	origin_base->clear();
	origin_base = nullptr;
}


size_t policy_base_server::get_policy_count( std::string trigger ) const
{
	if ( trigger.empty() )
		return base_map.size();

	size_t n = 0;
	trigger_base tr(trigger);
	for ( auto & it : base_map )
	{
		if ( tr.hit_trigger( it.second->m_triggle ) )
			n++;
	}

	return n;
}


policy_work_ptr policy_base_server::clone_policy( std::string name ) const
{
	auto it = base_map.find(name);
	if ( it == base_map.end() || nullptr == it->second )
		return nullptr;

	return policy_work_ptr( new policy_work(it->second->clone_one(),it->first));
}


policy_work_ptr policy_base_server::clone_policy( size_t no ) const
{
	if ( no >= base_map.size() )
		return nullptr;
	auto it = base_map.begin();
	while ( no-- > 0 )
		it++;

	if ( nullptr == it->second )
		return nullptr;
	
	return policy_work_ptr( new policy_work(it->second->clone_one(),it->first));
	
}



bool policy_base_server::get_policys( std::string trigger, std::vector<std::string>& policy_list) const
{
	if ( base_map.empty() )
		return false;
	
	policy_work_ptr ptr = nullptr;
	policy_list.clear();
	policy_list.reserve( base_map.size());

	if ( trigger.empty() )
	{
		for( auto & policy: base_map )
		{
			if ( policy.second == nullptr || policy.first.empty() )
				continue;

			policy_list.push_back(policy.first);
		};
	}
	else
	{
		trigger_base tr(trigger);
		for( auto & policy: base_map )
		{
			if ( policy.second == nullptr || policy.first.empty()  )
				continue;

			if ( false == tr.hit_trigger( policy.second->m_triggle ) )
				continue;

			policy_list.push_back(policy.first);
		};
	}

	return false == policy_list.empty();
};





void policy_base_server::free_policy( policy_work_ptr ptr )
{
	assert(ptr);
	if ( ptr == nullptr || nullptr == ptr->point()  )
		return ;
	auto it = base_map.find(ptr->name());
	if ( it != base_map.end() )
		it->second->close_one(ptr->point());
	else
	{
		origin_base->close_one(ptr->point());
	}
}




};




