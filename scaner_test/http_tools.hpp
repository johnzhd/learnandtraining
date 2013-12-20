#pragma once

#include <regex>

#include <string>
#include <sstream> 

#include <map>



namespace http_tools
{
	template <class T> 
	std::string ConvertToString(T value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	template<typename Package_Ptr, typename Content>
	size_t get_urls_from_package( Package_Ptr begin_, Package_Ptr end_, Content& out_ )
	{
		try
		{
			size_t n = 0;
			//href=
			//src=
			//<FORM METHOD=post ACTION="/names.nsf?Login"
			std::string pattern ("([^c]ei)");
			pattern = "([[:alpha:]]*)" + pattern + "([[:alpha:]]*)";

			std::regex r(pattern, std::regex::icase);
			for(std::sregex_iterator it(begin_, end_,r),end_it;
				it != end_it; ++it)
			{
				(*it).str(0);
				(*it).str(1); // frist "([[:alpha:]]*)"
				(*it).str(2); // "([^c]ei)"
				(*it).str(3); // second "([[:alpha:]]*)"
				out_.push_back( it->str() );
				n++;
			}
		}
		catch ( std::regex_error &e)
		{
		}
		catch (...)
		{
		};
		return n;
	};

	// make up
	enum send_package_type
	{
		GET = 0,
		POST,
		POST_MULTIPART
	};
	std::string get_package( send_package_type post, const std::string & url_, const std::string & head_ );

	// divil

	std::string get_protocal( const std::string & url_ );
	std::string get_domain( const std::string & url_ );
	std::string get_page( const std::string & url_ );
	std::string get_port( const std::string & url_ );
	bool get_url_info( const std::string & url_,
		std::string* protocal, std::string* domain, std::string* port, std::string* path,
		std::string* params);

	std::string get_port_by_protocal( std::string protocal );

	std::string format_url( std::string origin_url );

	std::string format_head(std::string head);
	
	bool is_ssl( const std::string& protocal );


	
	bool url_2_ip( const std::string& url, std::string ip, std::string port );

	template<typename Byte_Ptr>
	bool http_get_head( Byte_Ptr& begin_, Byte_Ptr end_,
		std::map<std::string,std::string>* head_info,
		size_t* state)
	{
		while ( begin_ != end_ )
		{
			if ( strncmp( reinterpret_cast<const char*>(&(*begin_)), "HTTP",4 ) == 0 )
				break;

			begin_++;
		}
		if ( begin_ == end_ )
			return false;

		begin_ += 4;
		Byte_Ptr p = end_, p1 = end_;
		int n = 0, n1 = 0;
		
		if ( state )
		{
			for ( begin_; begin_ != end_; begin_++ )
			{
				if ( *begin_ == ' ' )
				{
					if ( end_ == p )
						p = begin_ + 1;
					else
					{
						n = begin_ - p;
						break;
					}
				}
			}

			if ( end_ != p && 0 != n )
				*state = atoi( std::string(p,p+n).c_str() );
			else
				return false;
		}

		if ( !head_info )
		{
			return true;
		}

		for ( begin_; begin_ != end_ ; begin_++ )
		{
			if ( *begin_ == '\n' )
			{
				begin_++;
				break;
			}
		}

		p = p1 = end_;
		n = n1 = 0;
		for ( begin_; begin_ !=end_ ; begin_++ )
		{
			
			if ( strncmp( reinterpret_cast<const char*>( &(*begin_)), "\r\n\r\n",4 ) == 0 )
			{
				n1 = begin_ - p1;
				if ( p != end_ && p1 != end_ && n>0 && n1>0 )
				{
					std::string key_(p,p+n);
					std::string value_(p1,p1+n1);
					std::transform(key_.begin(),key_.end(),key_.begin(),tolower);
					value_.erase(0, value_.find_first_not_of(" \r\n\t;"));
					value_.erase(value_.find_last_not_of(" \r\n\t;")+1, std::string::npos);
					if ( !value_.empty() )
					{
						if ( (*head_info)[ key_ ].empty() )
							(*head_info)[ key_ ] = value_;
						else
							(*head_info)[ key_ ] += ";"+value_;
					}
				};
				p = p1 = end_;
				n = n1 = 0;

				begin_+=4;
				return true;
			}
			if ( strncmp(reinterpret_cast<const char*>(&(*begin_)), "\n\n",2 ) == 0 )
			{
				n1 = begin_ - p1;
				if ( p != end_ && p1 != end_ && n>0 && n1>0 )
				{
					std::string key_(p,p+n);
					std::string value_(p1,p1+n1);
					std::transform(key_.begin(),key_.end(),key_.begin(),tolower);
					value_.erase(0, value_.find_first_not_of(" \r\n\t;"));
					value_.erase(value_.find_last_not_of(" \r\n\t;")+1, std::string::npos);
					if ( !value_.empty() )
					{
						if ( (*head_info)[ key_ ].empty() )
							(*head_info)[ key_ ] = value_;
						else
							(*head_info)[ key_ ] += ";"+value_;
					}
				};
				p = p1 = end_;
				n = n1 = 0;

				begin_ +=2;
				return true;
			};

			if ( p == end_ )
				p = begin_;

			if ( p1 == end_ && *begin_ == ':' )
			{
				n = begin_ - p;
				p1 =begin_+1;
			}
			else if ( *begin_ == '\n' )
			{
				n1 = begin_ - p1;
				if ( p != end_ && p1 != end_ && n>0 && n1>0 )
				{
					std::string key_(p,p+n);
					std::string value_(p1,p1+n1);
					std::transform(key_.begin(),key_.end(),key_.begin(),tolower);
					value_.erase(0, value_.find_first_not_of(" \r\n\t;"));
					value_.erase(value_.find_last_not_of(" \r\n\t;")+1, std::string::npos);
					if ( !value_.empty() )
					{
						if ( (*head_info)[ key_ ].empty() )
							(*head_info)[ key_ ] = value_;
						else
							(*head_info)[ key_ ] += ";"+value_;
					}
				};
				p = p1 = end_;
				n = n1 = 0;
			}
		};

		head_info->clear();
		return false;

	};

	class http_info
	{
	public:
		http_info():status(0),head(),translate_wstr_body(L""),body_origin(){};
		~http_info(){};
		http_info( const http_info& src ){ *this = src;};
		http_info& operator=(const http_info& src)
		{
			status = src.status;
			head.clear();
			for ( auto it = src.head.begin(); it != src.head.end(); it++ )
			{
				head.insert(std::pair<std::string,std::string>(it->first,it->second));
			}

			body_origin.clear();
			body_origin.insert( body_origin.end(), src.body_origin.begin(), src.body_origin.end() );

			translate_wstr_body = src.translate_wstr_body;
			body_charset = src.body_charset;

			return *this;
		};
	public:
		size_t status;
		std::wstring translate_wstr_body;
		std::string body_charset;
		std::map<std::string,std::string> head;
		std::vector<unsigned char> body_origin;
	};

	bool http_reponse_check( std::vector<unsigned char>& content );
	bool http_reponse_completed( std::vector<unsigned char>& content, std::vector<http_info>& v_list, std::string& trigger );

	void get_new_cookie( std::string recv_head, std::string old_cookie, std::string& new_cookie );
};

