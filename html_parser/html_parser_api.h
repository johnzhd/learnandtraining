#pragma once

#include <vector>


#ifdef HTML_PARSER_EXPORTS

#define VPN_API_DllExport  extern "C"  __declspec( dllexport ) 

VPN_API_DllExport size_t API_html_parser(const char* body, std::vector<std::string>& v_out);

#else

typedef int (*func_html_parser_run)(const char* body, std::vector<std::string>& v_out);

#endif