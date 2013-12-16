#include "stdafx.h"
#include "policy_work.h"

#pragma comment(lib, "liblua.lib")

//////////////////////////////////////////////////////////////////////////////////////////
// API
//
int globle_func_insert_load(lua_State* lua)
{
	policy_api::policy_server_ptr ptr;
	ptr = policy_api::get_server();
	if ( ptr == 0 )
		return 0;
	std::string str;
	std::vector<std::string> v_str;
	str = lua_tostring(lua, -1);

	size_t npos;
	for ( npos = str.find("<|>"); npos != std::string::npos ; npos = str.find("<|>") )
	{
		v_str.push_back( str.substr(0,npos) );
		str.erase(0,npos+3);
	}
	if ( false == str.empty() )
		v_str.push_back( str );

	for ( auto it = v_str.begin(); it != v_str.end(); it++ )
	{
#ifdef _MSC_VER
		if ( std::string::npos == it->find(":\\") )
#else
		if ( it->substr(0,1) != "/" )
#endif
		{
			(*it) = POLICY_STR_GET_APP_PATH + (*it);
		};
		ptr->insert_plugins_name( *it );
	}

	return 0;
	
};


namespace policy_api
{
	trigger_base::trigger_base(std::string trigger)
	{
		assert(false == trigger.empty());
		if ( false == divil_trigger(trigger,v_trigger) )
		{
			//log
		}

	};
	trigger_base::~trigger_base()
	{
	};
	bool trigger_base::hit_trigger(std::string trigger)
	{
		if ( trigger.empty() )
			return false;
		for ( auto& str : v_trigger)
		{
			if( trigger.find( str ) != std::string::npos )
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
		v_t.clear();
		for ( nf = 0, nb = trigger.find("||",nf); nf < trigger.length(); nf = nb + 2, nb = trigger.find("||",nf))
		{
			str_temp = trigger.substr(nf,nb-nf);
			if ( str_temp.empty() )
				continue;

			std::transform(str_temp.begin(),str_temp.end(),str_temp.begin(),tolower);
			v_t.push_back(str_temp);
		};
		return false == v_t.empty();
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
};



policy_base_server::~policy_base_server()
{
	
};



bool policy_base_server::init()
{
	//origin_base new
	_ASSERT( origin_base == nullptr );

	try{


		//origin_base init function
		origin_base.reset(new policy_base() );

		// init_work load ini file
		if ( false == origin_base->load( (POLICY_STR_GET_APP_PATH + POLICY_STR_MAIN_FILE).c_str() ) )
			throw std::string("Load file ") +POLICY_STR_GET_APP_PATH + POLICY_STR_MAIN_FILE + " failed.";

		if ( false == origin_base->init_environment(policy_base::INIT_WHOLE) )
			throw std::string("Set policy environment failed.");

		// init_work run init call
		std::vector<std::string> v_in, v_out;
		v_in.resize(POLICY_INT_DEFAULT_IN_COUNT,"");
		v_out.resize(POLICY_INT_DEFAULT_OUT_COUNT,"");
		if ( false == origin_base->call_function(v_out, POLICY_STR_FUNCTION_FIRST_CALL, v_in) )
			throw v_out[0];
		// lua insert file list

		// check file list
		size_t n;
		n = base_map.size();
		std::vector<decltype(base_map.begin())> eraser_v;
		// loop load plugins
		for( auto it = base_map.begin(); it != base_map.end(); it++ )
		{
			it->second.reset(new policy_base());
			if ( false == it->second->load(it->first.c_str()) )
			{
				it->second = nullptr;
				eraser_v.push_back(it);
				// Log load policy it->first; failed
				// Log it->second->get_string_globle(POLICY_STR_UNION_ID); failed
				continue;
			}

			if ( false == it->second->init_environment() )
			{
				it->second = nullptr;
				eraser_v.push_back(it);
				continue;
			}

			// Log load policy it->first; 
			// Log it->second->get_string_globle(POLICY_STR_UNION_ID);
		}

		for ( auto it = eraser_v.begin(); it != eraser_v.end(); it++ )
		{
			base_map.erase( *it );
		}

		return true;
	}
	catch (std::string& e)
	{
		printf("%s %s %s\n",__FUNCTION__,__LINE__,e.c_str());
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


void policy_base_server::free_policy( policy_work_ptr ptr )
{
	if ( ptr == nullptr )
		return ;
	auto it = base_map.find(ptr->name());
	if ( it != base_map.end() )
		delete_copy(*ptr);
}

policy_work_ptr policy_base_server::get_policy( std::string name ) const
{
	auto it = base_map.find(name);
	if ( it == base_map.end() )
		return nullptr;
	if ( it->second )
		return create_copy(it->second, it->first);
	return nullptr;
}

policy_work_ptr policy_base_server::get_policy( size_t no ) const
{
	if ( no >= base_map.size() )
		return nullptr;
	auto it = base_map.begin();
	while ( no-- > 0 )
		it++;

	if ( it->second )
		return create_copy(it->second, it->first);
	return nullptr;
}



bool policy_base_server::get_policys( std::string trigger, std::vector<policy_work_ptr>& policy_list) const
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
			if ( policy.second == nullptr )
				continue;

			ptr = create_copy(policy.second, policy.first);
			if ( ptr != nullptr )
				policy_list.push_back(ptr);
		};
	}
	else
	{
		trigger_base tr(trigger);
		for( auto & policy: base_map )
		{
			if ( policy.second == nullptr )
				continue;

			if ( false == tr.hit_trigger( policy.second->m_triggle ) )
				continue;

			ptr = create_copy(policy.second, policy.first);
			if ( ptr != nullptr )
				policy_list.push_back(ptr);
		};
	}

	return policy_list.empty();
};





size_t policy_base_server::get_policy_count() const
{
	return base_map.size();
}

void policy_base_server::insert_plugins_name( std::string name )
{
	base_map.insert( std::pair<std::string,policy_base_ptr>(name,nullptr));
}

policy_work_ptr policy_base_server::create_copy(policy_base_ptr base, std::string name) const
{
	if ( base->empty() )
		return nullptr;
	lua_State* lu = lua_newthread(base->point());
	if ( lu == NULL )
	{
		return nullptr;
	}
	return policy_work_ptr( new policy_work(lu, name) );
};

void policy_base_server::delete_copy(policy_work& sub) const
{
	sub.clear();
};


//////////////////////////////////////////////////////////////////////////////////////////
// base class
//

policy_base::policy_base()
	:lu_ptr(NULL),
	m_version(""),
	m_vul_union_id(""),
	m_triggle("")
{
};

policy_base::~policy_base()
{
	clear();
};

lua_State*& policy_base::point()
{
	return lu_ptr;
}
bool policy_base::load(const char * filePathName)
{
	clear();
	lu_ptr = luaL_newstate();
	if ( lu_ptr == NULL )
		return false;
	luaL_openlibs(lu_ptr);

	// policy
	if ( LUA_OK != luaL_dofile(lu_ptr, filePathName) )
	{
		clear();
		return false;
	}

	if ( false == get_globle_value() )
	{
		clear();
		return false;
	}
	return true;
};

bool policy_base::empty() const
{
	return lu_ptr == NULL;
};

void policy_base::clear()
{
	m_version.clear();
	m_vul_union_id.clear();
	m_triggle.clear();
	if ( NULL != lu_ptr )
	{
		lua_close( lu_ptr );
		lu_ptr = NULL;
	};
};



bool policy_base::init_environment(init_environment_type type)
{
	// set 
	//lua_pushcfunction(lu_ptr, fcAdd);
	//lua_setglobal(lu_ptr, "fcAdd");
	// to be continue
	switch ( type )
	{
	case INIT_WORK:
		break;
	case INIT_WHOLE:
		set_function_globle(GLOBLE_FUNC_NAME_INSERT_LOAD, globle_func_insert_load);
		break;
	default:
		return false;
		break;
	}
	return true;
};

bool policy_base::get_globle_value()
{
	m_version = get_string_globle(POLICY_STR_VERSION);
	if ( m_version.empty() )
		return false;

	m_vul_union_id = get_string_globle(POLICY_STR_UNION_ID);
	if ( m_version.empty() )
		return false;

	m_triggle = get_string_globle(POLICY_STR_TRIGGLE);
	if ( m_version.empty() )
		return false;

	return true;
};

std::string policy_base::get_string_globle(const char* name)
{
	const char * p = NULL;
	lua_getglobal(lu_ptr, name);
	p = lua_tostring(lu_ptr, -1);
	lua_pop(lu_ptr, 1);

	return std::string(p?p:"");
}

int policy_base::get_int_globle(const char* name)
{
	int n = 0;
	lua_getglobal(lu_ptr, name);
	n = lua_tointeger(lu_ptr, -1);
	lua_pop(lu_ptr, 1);

	return n;
}

inline bool policy_base::set_function_globle( const char* name, lua_CFunction func)
{
	if ( nullptr == name || *name == '\0' || func == nullptr )
		return false;
	lua_pushcfunction(lu_ptr, func);  
	lua_setglobal(lu_ptr, name);
	return true;
}


bool policy_base::call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in )
{
	bool b;
	int nret;
	const char * p;

	lua_getglobal(lu_ptr, name);
	for ( size_t i = 0 ; i < v_in.size() ; i++ )
		lua_pushstring(lu_ptr, v_in[i].c_str());

	b = (LUA_OK == lua_pcall(lu_ptr, v_in.size(),v_out.size(),0));
	nret = lua_gettop(lu_ptr);
	nret = std::min(nret, static_cast<int>(v_out.size()));
	for( int i = 0; i < nret; i++ )
	{
		p = lua_tostring(lu_ptr,i+1);
		v_out[i] = p?p:"";
	}
	lua_pop(lu_ptr, nret);
	return b;
}

//////////////////////////////////////////////////////////////////////////////////////////
// work class
//

policy_work::policy_work(lua_State * lu, std::string str)
	:lu_ptr(lu),
	str_name(str)
{
}


policy_work::~policy_work(void)
{
}

lua_State*& policy_work::point()
{
	return lu_ptr;
};
std::string policy_work::name()
{
	return str_name;
}

void policy_work::clear()
{
	lu_ptr = NULL;
}

bool policy_work::empty() const
{
	return lu_ptr == NULL;
}

bool policy_work::call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in )
{
	bool b;
	int nret;
	const char * p;

	lua_getglobal(lu_ptr, name);
	for ( size_t i = 0 ; i < v_in.size() ; i++ )
		lua_pushstring(lu_ptr, v_in[i].c_str());

	b = (LUA_OK == lua_pcall(lu_ptr, v_in.size(),v_out.size(),0));
	nret = lua_gettop(lu_ptr);
	for( int i = 0; i < nret; i++ )
	{
		p = lua_tostring(lu_ptr,i+1);
		v_out[i] = p?p:"";
	}
	lua_pop(lu_ptr, nret);
	return b;
}

bool policy_work::run_demo()
{
	_ASSERT( 0 == lua_gettop(lu_ptr) );
	std::vector<std::string> v_in;
	std::vector<std::string> v_out;
	v_in.resize(3,"");
	v_out.push_back("Init_Lua");
	v_out.push_back("");
	v_out.push_back("");
	int i = 0;
	while ( false == v_out[0].empty() && true == call_function(v_out, v_out[0].c_str(), v_in) ) 
	{
		if ( i++ == 5 )
		{
			v_in[0] = "123";
		}
	}
	printf( "%s\n", v_out[2].c_str() );
	return true;
}
























};




