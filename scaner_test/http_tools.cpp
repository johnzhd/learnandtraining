#include "stdafx.h"
#include "http_tools.hpp"

#include <algorithm>
#include <map>

#include "gzip.h"
#include "charset.h"

#include "policy_work.h"

namespace http_tools
{
	std::string get_package( send_package_type post, const std::string & url_, const std::string & head_ )
	{
		if ( post == POST_MULTIPART )
		{
			_ASSERT(false);
			// call other
			return "";
		}
		std::string domain,port,path,params,str;
		if ( false == get_url_info( url_, NULL, &domain, &port, &path, &params ) )
			return "";

		switch ( post )
		{
		case POST:
			str = std::string("POST ") + (path.empty()? "/":path) + " HTTP/1.1\r\n";
			break;
		case GET:
		default:
			str = std::string("GET ") + (path.empty()? "/":path) + (params.empty()?"":"?") + params + " HTTP/1.1\r\n";
			break;
		};

		
		// 
		// Host
		// Accept: */*
		// Accept-Encoding: gzip, deflate
		if ( head_.find( "Host:" ) == std::string::npos )
			str += "Host: " + domain + port + "\r\n";
		if ( head_.find( "Accept:" ) == std::string::npos )
			str += "Accept: */*\r\n";
		if ( head_.find( "Accept-Encoding:" ) == std::string::npos )
			str += "Accept-Encoding: gzip, deflate\r\n";
		if ( head_.find( "User-Agent:" ) == std::string::npos )
			str += "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0; Trident/5.0)\r\n";
		if ( head_.find( "Connection:" ) == std::string::npos )
			str += "Connection: keep-Alive\r\n";
		

		if ( post == POST )
		{
			if ( head_.find( "Content-Type:" ) == std::string::npos )
				str += "Content-Type: application/x-www-form-urlencoded\r\n";
			// Content-type: multipart/form-data; boundary=------------TXFTNActiveX.Package.Partition.477953412.2660834128.30335057
			if ( head_.find( "Content-Length:" ) == std::string::npos )
				str += "Content-Length: " + ConvertToString(params.length()) + "\r\n";
		}

		str += head_ + "\r\n";
		

		if ( post )
		{
			str += params;
		};

		return str;
	}
}


namespace http_tools
{
	bool get_url_info( const std::string & url_,
		std::string* protocal, std::string* domain, std::string* port, std::string* path,
		std::string* params)
	{
		if ( url_.empty() )
		{
			return false;
		}
		std::string strDomain(url_),strProtocal;

		size_t npos;
		npos = strDomain.find("://");
		if ( npos != std::string::npos )
		{
			strProtocal = strDomain.substr(0,npos);
			strDomain.erase(0, npos + 3);
		}
		else
		{
			strProtocal = "http";
		}

		if ( protocal )
			*protocal = strProtocal;

		npos = strDomain.find_first_of("?#");
		if ( npos != std::string::npos )
		{
			if ( params )
				*params = strDomain.substr(npos);
			strDomain.erase( npos );
		};

		npos = strDomain.find_first_of("/");
		if ( npos != std::string::npos )
		{
			if ( path )
				*path = strDomain.substr(npos);
			strDomain.erase( npos );
		}
		else
		{
			if ( path )
				*path = "/";
		}

		npos = strDomain.find(':');
		if ( npos != std::string::npos )
		{
			if ( port )
				*port = strDomain.substr(npos);
			strDomain.erase( npos );
		}
		else
		{
			if ( port )
				*port = ":" + get_port_by_protocal(strProtocal);
		}

		if ( domain )
			*domain = strDomain;

		return true;
	};

	std::string format_url(std::string url)
	{
		std::string str[5];
		if ( false == http_tools::get_url_info(url,&str[0],&str[1],&str[2],&str[3],&str[4]) )
			return "";

		return str[0]+"://"+str[1]+str[2]+str[3]+str[4];
	};

	std::string format_head(std::string head)
	{
		size_t npos;
		npos = head.find_first_not_of("\r\n \t");
		head.erase(0,npos);
		npos = head.find_last_not_of("\r\n \t");
		if( npos != std::string::npos )
			head.erase(npos+1);

		if ( head.empty())
			return "";

		head += "\r\n";
		return head;
	};

	std::string get_port_by_protocal( std::string protocal )
	{
		std::transform(protocal.begin(),protocal.end(),protocal.begin(),tolower);
		switch (protocal.length())
		{
		case 3:
			if ( protocal.compare("ftp") == 0 )
				return "21";
			else if ( protocal.compare("ssh") == 0 )
				return "22";
			break;
		case 4:
			if ( protocal.compare("http") == 0 )
				return "80";
			break;
		case 5:
			if ( protocal.compare("https") == 0 )
				return "443";
			break;
		default:
			break;
		};
		return "80";
	};

	std::string get_protocal( const std::string & url_ )
	{
		auto npos = url_.find("://");
		if ( npos == std::string::npos )
			return "http";

		std::string str( url_.substr(0,npos) );
		std::transform(str.begin(),str.end(),str.begin(),tolower);
		return str;
	};

	
	std::string get_domain( const std::string & url_ )
	{
		auto npos = url_.find("://");
		if ( npos == std::string::npos )
			npos = 0;
		else
			npos += 3;
		decltype(npos) count;
		count = url_.find_first_of(":/?#", npos );

		return url_.substr( npos, count-npos);
	};

	std::string get_page( const std::string & url_ )
	{
		auto npos = url_.find("://");
		if ( npos == std::string::npos )
			npos = 0;
		else
			npos += 3;
		npos = url_.find_first_of("/?#", npos );
		if ( npos == std::string::npos )
			return url_;

		return url_.substr(0,npos)+"/";
	};
	std::string get_port( const std::string & url_ )
	{
		auto npos = url_.find(':');
		if ( npos == std::string::npos )
			return "80";

		std::string str;


		if ( url_[npos+1] == '/' && url_[npos+2] == '/' )
		{
			str = url_.substr(0,npos);
			npos = url_.find_first_of(':', npos+3);
			if ( npos == std::string::npos )
				return is_ssl(str) ? "443" : "80";
		};

		decltype(npos) count = 0;
		npos++;
		while ( npos+count < url_.length() && isdigit(url_[npos+count])!=0 )
		{
			count++;
		};
		return url_.substr(npos,count);				
	};

	bool is_ssl( const std::string& protocal )
	{
		return _stricmp( protocal.c_str(), "https" ) == 0;
	};
};

namespace http_tools
{
	bool http_reponse_check( std::vector<unsigned char>& content )
	{
		if ( content.empty() )
			return false;

		auto it = content.begin();
		auto itWork = content.begin();
		auto itEnd = content.end();
		
		// deal head
		std::map<std::string,std::string> head;
		size_t status;
		// deal body
		while ( itWork != itEnd )
		{
			if (false == http_get_head( itWork,itEnd,&head,&status))
			{
				return false;
			}

			it = itWork;
			

			if ( status == 100 )
			{
				while ( itWork!=itEnd && strncmp( reinterpret_cast<const char*>(&(*itWork++)), "\nHTTP/", strlen("\nHTTP/") ) )
				{
					continue;
				}
				if ( itWork != itEnd )
				{
					continue;
				}
				return false;
			};

			// to be continue

			if ( false == head["content-length"].empty() )
			{
				int nLength = atoi(head["content-length"].c_str() );
				if ( itEnd - itWork >= nLength )
					return true;
			}
			else if ( false == head["transfer-encoding"].empty()
				&& head["transfer-encoding"].find( "chunked" ) != std::string::npos )
			{
				it = itWork;
				while ( itWork != itEnd )
				{
					if ( *itWork++ == '\n' )
					{
						char * p;
						auto nlength = strtol( reinterpret_cast<char*>(&*it), &p, 16 );
						if ( nlength > itEnd - itWork )
							return false;
						
						
						if ( 0 == nlength && itWork < itEnd )
						{
							return true;
						}
						it = itWork + nlength + 2; // jump [{data}\r\n]
						itWork = it;
					}
				}

			}

			while ( itWork != itEnd )
			{
				if ( 0 == strncmp(reinterpret_cast<const char*>(&(*itWork++)), "\nHTTP/", strlen("\nHTTP/")) )
				{
					break;
				}
			}



		}
		return false;

	};
	bool http_reponse_completed( std::vector<unsigned char>& content, std::vector<http_info>& v_list, std::string& trigger)
	{
		if ( content.empty() )
			return false;
		// 100 jump to 200
		// 200 check Content-length
			// check \r\n\r\n
			// count length
			// return 
		// 200 check Transfer-Encoding = chunked
			// check \r\n\r\n mark npos
			// check \r\n count npos
			// jump npos mark new npos
			// check \r\n count npos
			// jump ...
			// return
		// other check \r\n\r\n
		// return 

		// Get Head
		auto it_package = content.begin();
		auto it = content.begin();
		auto itWork = content.begin();
		auto itEnd = content.end();

		v_list.clear();

		// deal head

		// deal body
		while ( itWork != itEnd )
		{
			v_list.push_back(http_info());
			auto v_it = v_list.rbegin();

			auto& v = *v_it;
			it_package = itWork;
			if (false == http_get_head( itWork,itEnd,&v.head,&v.status))
			{
				return false;
			}

			it = itWork;
			

			if ( v.status == 100 )
			{
				while ( itWork!=itEnd && strncmp( reinterpret_cast<const char*>(&(*itWork++)), "\nHTTP/", strlen("\nHTTP/") ) )
				{
					v.body_origin.insert( v.body_origin.end(), it, itWork );
					continue;
				}
			};

			// to be continue
			decltype( v.head.begin() ) it_head;
			if ( v.head.end() != (it_head = v.head.find("content-length"))  && false == it_head->second.empty()  )
			{
				int nLength = atoi( it_head->second.c_str() );
				if ( itEnd - itWork >= nLength && nLength > 0 )
				{
					v.body_origin.insert(v.body_origin.end(), itWork, itWork + nLength );
				}
			}
			else if ( v.head.end() != (it_head = v.head.find("transfer-encoding")) 
				&& false == it_head->second.empty()
				&& it_head->second.find( "chunked" ) != std::string::npos )
			{
				it = itWork;
				while ( itWork != itEnd )
				{
					if ( *itWork++ == '\n' )
					{
						char * p;
						auto nlength = strtol( reinterpret_cast<char*>(&*it), &p, 16 );
						if ( nlength > itEnd - itWork )
							return false;
						
						
						if ( 0 == nlength )
						{
							itWork += 2; // jump last \r\n at tail;
							break;
						}

						v.body_origin.insert( v.body_origin.end(), itWork, itWork + nlength );

						it = itWork + nlength + 2; // jump [{data}\r\n]
						itWork = it;
					}
				}
			}

			it = itWork;
			while ( itWork != itEnd )
			{
				if ( 0 == strncmp(reinterpret_cast<const char*>(&(*itWork++)), "\nHTTP/", strlen("\nHTTP/")) )
				{
					break;
				}
			}




			if ( it != itWork )
				v.body_origin.insert(v.body_origin.end(), it, itWork );

			it_package = itWork;

			// deal body
			if ( v.head.end() != (it_head = v.head.find("content-encoding")) && false == it_head->second.empty() )
			{
				if ( it_head->second.find( "gzip" ) != std::string::npos )
				{
					decltype(v.body_origin) v_temp;
					if ( true == gzip::ungzip( v.body_origin, v_temp) )
					{
						v.body_origin.swap( v_temp );
					}
					else
					{
						// unzip err
					}
				}
			}

			size_t n_temp = 0;
			v.body_charset.clear();
			if ( v.head.end() != (it_head = v.head.find("content-type")) && false == it_head->second.empty() )
			{
				if ( it_head->second.find( "text/" ) != std::string::npos )
				{
					if ((n_temp = it_head->second.find("charset=")) != std::string::npos)
					{
						n_temp += strlen( "charset=" );
						std::string str;
						str = it_head->second.substr( n_temp );
						if ( std::string::npos != (n_temp = str.find_first_of( " ;\r\n" )) )
							str.erase(n_temp);

						std::transform( str.begin(), str.end(), str.begin(), tolower );

						v.body_charset = str;
					}


					if ( v.body_charset.empty() )
						v.body_charset = "gbk";

					charset::any_to_unicode( v.body_charset, v.body_origin, v.body_origin.size(), v.translate_wstr_body );

					trigger = policy_api::trigger_base::format_trigger(trigger, "once");
				}
			}
			
			//else No content-length No chunked
		};

		return true;

	};

	static inline void trim( std::string& str )
	{
		str.erase(0,str.find_first_not_of("\r\n \t"));
		str.erase(str.find_last_not_of("\r\n \t") + 1,std::string::npos);
	};

	static void cookie_2_map( std::string src, std::map<std::string,std::string>& map_out,
		std::string start_line, std::string end_line )
	{
		std::string str_temp,str_key,str_value;

		size_t nF,nB = 0, lf, lb,li;

		while ( std::string::npos != (nF=src.find(start_line,nB+end_line.length())) )
		{
			nF += start_line.size();
			nB = src.find(end_line,nF);
			if ( nB == std::string::npos )
				nB = src.length();

			str_temp = src.substr(nF,nB-nF);

			trim(str_temp);

			if (str_temp.empty())
				continue;

			for ( lf = 0, lb = str_temp.find(";",lf) ; lf < str_temp.length() ; lf = lb + 1, lb = str_temp.find(";",lf) )
			{
				if ( lb == std::string::npos )
				{
					lb = str_temp.length();
				};
				li = str_temp.find("=");
				if (li == std::string::npos)
					continue;
				str_key = str_temp.substr(lf,li-lf);
				str_value = str_temp.substr( li+1, lb);

				trim(str_key);
				trim(str_value);
				if ( str_key.empty() || str_value.empty() )
					continue;

				map_out[str_key] = str_value;
			};
		}
	};

	static void map_2_cookie(std::map<std::string,std::string>& map_in, std::string& cookie_out)
	{
		if ( map_in.empty() )
			return ;
		size_t nline = 0;
		for ( auto it : map_in )
		{
			if ( 0 == nline )
			{
				cookie_out += "Cookie: " + it.first + "=" + it.second + ";";
				nline = it.first.length() + it.second.length() + 10;
			}
			else if ( (nline + it.first.length() + it.second.length()) < 80 )
			{
				cookie_out += it.first + "=" + it.second + ";";

				nline += it.first.length() + it.second.length() + 2;
			}
			else
			{
				cookie_out += "\r\nCookie: " + it.first + "=" + it.second + ";";
				nline = it.first.length() + it.second.length() + 10;
			}
		};

		cookie_out += "\r\n";
	};
	
	void get_new_cookie( std::string recv_head, std::string old_cookie, std::string& new_cookie )
	{
		if ( recv_head.find("Set-Cookie:") == std::string::npos )
		{
			new_cookie = old_cookie;
			return ;
		}

		std::map<std::string,std::string> map_temp;

		cookie_2_map(old_cookie, map_temp, "Cookie:", "\n");

		cookie_2_map(recv_head, map_temp, "Set-Cookie:", "\n");

		std::vector<std::string> v_erase;
		for (auto it : map_temp )
		{
			if ( it.second.empty() )
				v_erase.push_back(it.first);
		}

		for (auto str : v_erase )
		{
			map_temp.erase( str );
		};
		new_cookie.clear();
		map_2_cookie(map_temp, new_cookie);
	};

};