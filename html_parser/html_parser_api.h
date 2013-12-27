#pragma once

#include <set>
#include <string>

#ifdef HTML_PARSER_EXPORTS

#define VPN_API_DllExport  extern "C"  __declspec( dllexport ) 



#else

#define VPN_API_DllExport  extern "C"  __declspec( dllimport ) 


#endif

VPN_API_DllExport size_t API_html_parser(const char* p_body, size_t size_body, std::set<std::string>& v_out,
										   std::string page_url, size_t domain_sub_level);