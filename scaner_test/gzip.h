#pragma once

#include <vector>

namespace gzip
{
	bool ungzip(std::vector<unsigned char>& in, size_t count_in, std::vector<unsigned char>& out, size_t& count_out);
};

