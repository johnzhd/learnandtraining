#include "stdafx.h"
#include "html_parser_api.h"


#include <regex>
#include <algorithm>

#include "gumbo.h"
namespace html_parser
{

static inline std::string get_domain( const std::string & url_ )
{
	auto npos = url_.find("://");
	if ( npos == std::string::npos )
		npos = 0;
	else
		npos += 3;
	decltype(npos) count;
	count = url_.find_first_of(":/?#", npos );

	std::string str( url_.substr( npos, count-npos) );
	return str;
};



static inline bool match_domain( std::string domain_base, std::string domain2, size_t domain_level )
{
	auto it1 = domain_base.rbegin();
	auto it2 = domain2.rbegin();
	size_t dot_count = 0;
	for ( it1 , it2 ; it1 != domain_base.rend() && it2 != domain2.rend(); it1++,it2++ )
	{
		if ( *it1 != *it2 )
		{
			return dot_count >= domain_level;
		}

		if ( *it1 == '.' )
			dot_count++;
	};

	return it1 == domain_base.rend();
};

static void search_for_links(GumboNode*root, std::set<std::string>& v_out, std::string& page_url, size_t domain_level)
{
	GumboAttribute* href;
	switch ( root->v.element.tag )
	{
	case GUMBO_TAG_A:
		{
			if ( nullptr != (href = gumbo_get_attribute(&root->v.element.attributes, "href")) )
			{
				std::string str(href->value);
				if ( str.empty() )
					break;

				if ( false == page_url.empty() )
				{
					if ( str.find("://") == std::string::npos )
					{
						str = page_url+str;
					}
					else if ( false == match_domain(get_domain(page_url), get_domain(str), domain_level) )
					{
						break;
					};
				}


				v_out.insert(str);
			};
		}
		break;
	case GUMBO_TAG_FORM:
		break;
	default:
		break;
	};

	GumboVector* children = &root->v.element.children;
	if ( children == nullptr )
	{
		return ;
	}
	for (unsigned int i = 0; i < children->length; ++i)
	{
		if ( static_cast<GumboNode*>(children->data[i])->type != GUMBO_NODE_ELEMENT )
			continue;
		search_for_links(static_cast<GumboNode*>(children->data[i]),v_out, page_url,domain_level);
	}

};

size_t API_html_parser(const char* p_body, size_t size_body, std::set<std::string>& v_out,
						 std::string str_domain, size_t domain_sub_level)
{
	size_t s = v_out.size();


	GumboOutput* output = gumbo_parse_with_options(&kGumboDefaultOptions, p_body, size_body);


	if (output->root->type == GUMBO_NODE_ELEMENT) {
		search_for_links(output->root,v_out,str_domain,domain_sub_level);
	}
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return v_out.size() - s;
};

}