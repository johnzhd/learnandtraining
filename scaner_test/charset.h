#pragma once

#include <vector>
#include <string>

namespace charset
{




	bool utf8_to_gbk( std::vector<unsigned char>& in, size_t count_in, std::vector<unsigned char>& out, size_t& count_out );


	bool any_to_unicode( std::string charset_http, std::vector<unsigned char>& in, size_t count_in, std::vector<wchar_t>& out, size_t& count_out );
	bool any_to_unicode( std::string charset_http, std::vector<unsigned char>& in, size_t count_in, std::wstring& out);

	std::string to_acsII( std::wstring wstr );
	std::string to_acsII( std::vector<wchar_t>& wstr );
	std::wstring to_unicode( std::string str );

	bool transform( char * char_ptr, size_t& char_size, wchar_t * wchar_ptr, size_t& wchar_size, unsigned int Codepage = 0, bool bA2W = true );
};

