#pragma once

#include <set>
#include <string>

#ifdef HTML_PARSER_EXPORTS

#define VPN_API_DllExport  extern "C"  __declspec( dllexport ) 

VPN_API_DllExport size_t API_html_parser(const std::string& body, std::set<std::string>& v_out, std::string page_url);

VPN_API_DllExport size_t API_html_parser_1(const char* p_body, size_t size_body, std::set<std::string>& v_out, std::string page_url);

#else

typedef int (*func_html_parser_run)(const std::string& body, std::set<std::string>& v_out, std::string page_url);

#endif