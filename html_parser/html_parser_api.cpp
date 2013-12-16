#include "stdafx.h"
#include "html_parser_api.h"


#include "gumbo.h"

static void search_for_links(GumboNode*root, std::vector<std::string>& v_out)
{
	if (root->type != GUMBO_NODE_ELEMENT) {
		return ;
	}
	GumboAttribute* href;
	switch ( root->v.element.tag )
	{
	case GUMBO_TAG_A:
		{
			if ( nullptr != (href = gumbo_get_attribute(&root->v.element.attributes, "href")) )
			{
				v_out.push_back(href->value);
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
	for (unsigned int i = 0; i < children->length; ++i) {
		search_for_links(static_cast<GumboNode*>(children->data[i]),v_out);
	}
};

size_t API_html_parser(const char* body, std::vector<std::string>& v_out)
{
	v_out.clear();
	GumboOutput* output = gumbo_parse(body);
	search_for_links(output->root,v_out);
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return v_out.size();
};