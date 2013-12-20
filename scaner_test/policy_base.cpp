#include "stdafx.h"
#include "policy_base.h"


#include "policy_work.h"

// globle
namespace policy_api
{

int globle_func_insert_load(lua_State* lua)
{
	policy_api::policy_server_ptr ptr;
	ptr = policy_api::get_server();
	if ( ptr == nullptr )
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








}; // end namespace policy_api globle








// class base
namespace policy_api
{



policy_base::policy_base(init_environment_type type)
	:m_version(""),
	m_vul_union_id(""),
	m_triggle(""),
	lua_mutex(),
	m_type(type)
{
};

policy_base::~policy_base()
{
	clear();
};

char* policy_base::buff()
{
	if ( m_buff.empty() )
		return nullptr;

	return &m_buff[0];
};

size_t policy_base::buff_size()
{
	return m_buff.size();
};

std::string policy_base::name()
{
	return m_name;
};

bool policy_base::load(const char * filePathName)
{
	clear();
	std::vector<char> v_temp;
	if ( false == file_base_tools::load_file(filePathName,v_temp) )
	{
		clear();
		return false;
	};

	lua_State *ls_lua;
	ls_lua = luaL_newstate();
	if ( ls_lua == nullptr )
	{
		clear();
		return false;
	};

	// policy
	if ( LUA_OK != (luaL_loadbuffer(ls_lua, &v_temp[0], v_temp.size(), "plugin") || lua_pcall(ls_lua, 0, LUA_MULTRET, 0)) )
	{
		clear();
		return false;
	}

	m_version = get_string_globle(ls_lua,POLICY_STR_VERSION);
	if ( m_version.empty() )
	{
		clear();
		return false;
	};

	m_vul_union_id = get_string_globle(ls_lua,POLICY_STR_UNION_ID);
	if ( m_version.empty() )
	{
		clear();
		return false;
	};

	m_triggle = get_string_globle(ls_lua,POLICY_STR_TRIGGLE);
	if ( m_version.empty() )
	{
		clear();
		return false;
	};

	m_name = filePathName;
	close_one( ls_lua );
	ls_lua = nullptr;
	boost::mutex::scoped_lock lock(lua_mutex);
	m_buff.swap(v_temp);
	return true;
	
};

bool policy_base::empty() const
{
	return m_buff.empty();
};

void policy_base::clear()
{
	boost::mutex::scoped_lock lock(lua_mutex);
	m_version.clear();
	m_vul_union_id.clear();
	m_triggle.clear();
	m_buff.clear();
};

lua_State * policy_base::clone_one()
{
	if ( empty() )
		return nullptr;
	lua_State * ls_lua;
	ls_lua = luaL_newstate();
	if ( ls_lua == nullptr )
	{
		return nullptr;
	};
	
	luaL_openlibs(ls_lua);
	{
		boost::mutex::scoped_lock lock(lua_mutex);
		// policy
		if ( LUA_OK != (luaL_loadbuffer(ls_lua, buff(), buff_size(), "plugin") || lua_pcall(ls_lua, 0, LUA_MULTRET, 0)) )
		{
			close_one(ls_lua);
			return nullptr;
		}
	}
	
	if ( false == init_environment( ls_lua, m_type ) )
	{
		close_one(ls_lua);
		return nullptr;
	}
	
	return ls_lua;
};

void policy_base::close_one(lua_State * lua)
{
	if ( lua == NULL )
		return ;
	lua_close(lua);
};


bool policy_base::init_environment(lua_State *ls_lua, init_environment_type type)
{
	switch ( type )
	{
	case INIT_WORK:
		break;
	case INIT_WHOLE:
		set_function_globle(ls_lua, GLOBLE_FUNC_NAME_INSERT_LOAD, globle_func_insert_load);
		break;
	default:
		return false;
		break;
	}
	return true;
};


std::string policy_base::get_string_globle(lua_State * ls_lua, const char* name)
{
	assert(ls_lua);
	assert(name);
	const char * p = NULL;
	lua_getglobal(ls_lua, name);
	p = lua_tostring(ls_lua, -1);
	lua_pop(ls_lua, 1);

	return std::string(p?p:"");
}

int policy_base::get_int_globle(lua_State * ls_lua, const char* name)
{
	assert(ls_lua);
	assert(name);
	int n = 0;
	lua_getglobal(ls_lua, name);
	n = lua_tointeger(ls_lua, -1);
	lua_pop(ls_lua, 1);

	return n;
}

inline bool policy_base::set_function_globle(lua_State * ls_lua,  const char* name, lua_CFunction func)
{
	assert(ls_lua);
	assert(name);
	assert(func != nullptr);

	if ( nullptr == name || *name == '\0' || func == nullptr )
		return false;
	lua_pushcfunction(ls_lua, func);  
	lua_setglobal(ls_lua, name);
	return true;
}



}; // end namespace policy_api class base



namespace policy_api
{


policy_work::policy_work(lua_State * lu, std::string str)
	:lu_ptr(lu),
	str_name(str),
	lua_mutex()
{
}


policy_work::~policy_work(void)
{
	clear();
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
	lu_ptr = nullptr;
}

bool policy_work::empty() const
{
	return lu_ptr == nullptr;
}

bool policy_work::call_function( std::vector<std::string>& v_out, const char * name, const std::vector<std::string>& v_in )
{
	if ( lu_ptr == nullptr )
		return false;
	bool b;
	int nret;
	const char * p;
	boost::mutex::scoped_lock lock(lua_mutex);
	lua_getglobal(lu_ptr, name);
	for ( size_t i = 0 ; i < v_in.size() ; i++ )
		lua_pushstring(lu_ptr, v_in[i].c_str());

	b = (LUA_OK == lua_pcall(lu_ptr, v_in.size(),v_out.size(),0));
	nret = lua_gettop(lu_ptr);
	nret = std::min(nret, static_cast<int>(v_out.size()));
	assert(nret == 3 );
	for( int i = 0; i < nret; i++ )
	{
		p = lua_tostring(lu_ptr,i+1);
		v_out[i] = p?p:"";
	}
	lua_pop(lu_ptr, nret);
	return b;
}

}; // end namespace policy_api class work