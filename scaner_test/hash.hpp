#pragma once

template<typename Char_ptr>
size_t strhashkey( Char_ptr key, size_t max )
{
	size_t h = 0;
	size_t hl,hr;

	while ( *key )
	{
		h += *key;
		hl = 0x5C5 ^ ((h&0xFFF00000)>>18);
		hr = h&0x000FFFFF;
		h = hl^hr^ *key++;
	};
	return h % max;
}