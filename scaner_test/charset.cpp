#include "stdafx.h"
#include "charset.h"


#ifdef _MSC_VER
#include <Windows.h>
#else
#endif

namespace charset
{
	bool utf8_to_gbk( std::vector<unsigned char>& in, size_t count_in, std::vector<unsigned char>& out, size_t& count_out )
	{
		std::string str_temp(in.begin(), in.begin()+count_in);

		std::vector<char> v_temp; 

#ifdef _MSC_VER
		int len=0;  
		wchar_t *w_string;  
		char *utf8_string;  
		//������ansiת��Ϊunicode��unicode����ĳ���  
		len=MultiByteToWideChar(CP_UTF8,0,str_temp.c_str(), -1, NULL,0);//CP_ACPָʾ��ת��Ϊunicode����ı�������  
		v_temp.resize(2*len+2,'\0');
		w_string=reinterpret_cast<wchar_t *>(&v_temp[0]);
		//ansi��unicodeת��  
		MultiByteToWideChar(CP_UTF8, 0, str_temp.c_str(),-1,w_string, len);//CP_ACPָʾ��ת��Ϊunicode����ı�������  

		//����unicodeת��Ϊutf8��utf8����ĳ���  
		count_out = WideCharToMultiByte(CP_ACP, 0, w_string, -1, NULL, 0, NULL, NULL);//CP_UTF8ָʾ��unicodeת��Ϊ������  
		out.resize(count_out+1,'\0');
		utf8_string=reinterpret_cast<char *>(&out[0]);
		//unicode��utf8ת��
		count_out = WideCharToMultiByte(CP_ACP, 0, w_string, -1, utf8_string, count_out, NULL,NULL);//CP_UTF8ָʾ��unicodeת��Ϊ������  
		utf8_string[count_out]='\0';

		return true;  
#else
		//�����Ƚ�gbk����ת��Ϊunicode����  
		if(NULL==setlocale(LC_ALL,"zh_CN.utf8"))//����ת��Ϊunicodeǰ����,��ǰΪgbk����  
		{  
			printf("Bad Parameter\n");  
			return false;  
		}  

		int unicodeLen=mbstowcs(NULL,str_temp.c_str(),0);//����ת����ĳ���  
		if(unicodeLen<=0)  
		{  
			printf("Can not Transfer!!!\n");  
			return false;  
		}
		v_temp.resize( (unicodeLen+1)*2,'\0' );
		wchar_t *unicodeStr=interpret_cast<wchar_t *>(&v_temp[0]);  
		mbstowcs(unicodeStr, str_temp.c_str(), str_temp.length() );//��gbkת��Ϊunicode  

		//��unicode����ת��Ϊutf8����  
		if(NULL==setlocale(LC_ALL,"zh_CN.gbk"))//����unicodeת�������,��ǰΪutf8  
		{  
			printf("Bad Parameter\n");  
			return false;  
		}  
		int utfLen=wcstombs(NULL,unicodeStr,0);//����ת����ĳ���  
		if(utfLen<=0)  
		{  
			printf("Can not Transfer!!!\n");  
			return false;  
		}
		out.resize( utfLen + 1,'\0' );
		char* utfStr = interpret_cast<char *>(&out[0]);  
		wcstombs(utfStr,unicodeStr,utfLen);  
		utfStr[utfLen]=0;//��ӽ�����  
		return true;  


#endif
	}

#ifdef _MSC_VER
	UINT string_to_codepage( std::string charset_http )
	{
		std::transform( charset_http.begin(),charset_http.end(),charset_http.begin(),tolower);

		if ( charset_http == "utf8" || charset_http == "utf-8" )
		{
			return CP_UTF8;
		}
		//else if ( charset_http == "gbk" || charset_http == "gb2312" )
		{
			return CP_ACP;
		}
	};
#else

#endif

	bool any_to_unicode( std::string charset_http, std::vector<unsigned char>& in, size_t count_in, std::vector<wchar_t>& out, size_t& count_out )
	{
		std::string str_temp(in.begin(), in.begin()+count_in);

		size_t len;
		UINT Codepage;
		Codepage = string_to_codepage(charset_http);


#ifdef _MSC_VER 
		//������ansiת��Ϊunicode��unicode����ĳ���  
		len=MultiByteToWideChar(Codepage,0,str_temp.c_str(), -1, NULL,0);//CP_ACPָʾ��ת��Ϊunicode����ı�������  
		out.resize(len+1,'\0');
		//ansi��unicodeת��  
		count_out = MultiByteToWideChar(Codepage, 0, str_temp.c_str(),-1,&out[0], len);//CP_ACPָʾ��ת��Ϊunicode����ı�������
		out[count_out] = _T('\0');
		return len == count_out;
#else
#endif

	};

	bool any_to_unicode( std::string charset_http, std::vector<unsigned char>& in, size_t count_in, std::wstring& out )
	{
		std::string str_temp(in.begin(), in.begin()+count_in);

		size_t len;
		UINT Codepage;
		Codepage = string_to_codepage(charset_http);

		std::vector<wchar_t> w_v;


#ifdef _MSC_VER 
		//������ansiת��Ϊunicode��unicode����ĳ���  
		len=MultiByteToWideChar(Codepage,0,str_temp.c_str(), -1, NULL,0);//CP_ACPָʾ��ת��Ϊunicode����ı�������  
		w_v.resize(len+2,0);
		//ansi��unicodeת��  
		MultiByteToWideChar(Codepage, 0, str_temp.c_str(),-1,&w_v[0], len);//CP_ACPָʾ��ת��Ϊunicode����ı�������
		out.assign(w_v.begin(),w_v.end());
		return out.length() == len;
#else
#endif

	};


	std::string to_acsII( std::wstring wstr )
	{
		std::string str;
		size_t n1,n2;
		n1 = wstr.length()*2;
		str.resize( n1 , 0 );
		n2 = wstr.length();
		if ( false == charset::transform(&str[0],n1,&wstr[0],n2,CP_ACP,false ))
		{
			str.clear();
		}
		return str;
	}

	std::string to_acsII( std::vector<wchar_t>& wstr )
	{
		std::string str;
		size_t n1,n2;
		n1 = wstr.size()*2;
		str.resize( n1 , 0 );
		n2 = wstr.size();
		if ( false == charset::transform(&str[0],n1,&wstr[0],n2,CP_ACP,false ))
		{
			str.clear();
		}
		return str;
	}

	std::wstring to_unicode( std::string str )
	{
		std::wstring wstr;
		size_t n1,n2;
		n1 = str.length();
		n2 = str.length();
		wstr.resize( n2, 0 );
		if ( false == charset::transform(&str[0],n1,&wstr[0],n2,CP_ACP,true ))
		{
			wstr.clear();
		}
		return wstr;
	}













	bool transform( char * char_ptr, size_t& char_size, wchar_t * wchar_ptr, size_t& wchar_size, UINT Codepage, bool bA2W )
	{
		size_t len;
		if ( bA2W )
		{
#ifdef _MSC_VER

			//������ansiת��Ϊunicode��unicode����ĳ���  
			len=MultiByteToWideChar(Codepage,0,char_ptr, char_size, NULL,0);//CP_ACPָʾ��ת��Ϊunicode����ı������� 
			//ansi��unicodeת��  
			wchar_size = MultiByteToWideChar(Codepage, 0, char_ptr,char_size,&wchar_ptr[0], wchar_size);//CP_ACPָʾ��ת��Ϊunicode����ı�������
			return wchar_size == len;
#else
#endif
		}
		else
		{
			BOOL b = FALSE;
			len=WideCharToMultiByte(Codepage,0,wchar_ptr, wchar_size, NULL,0,NULL,&b);//CP_ACPָʾ��ת��Ϊunicode����ı�������  
			//ansi��unicodeת��  
			b = TRUE;
			char_size=WideCharToMultiByte(Codepage, 0, wchar_ptr,wchar_size,&char_ptr[0], len,"*", &b);//CP_ACPָʾ��ת��Ϊunicode����ı�������
			return char_size == len;
		}
	};
};