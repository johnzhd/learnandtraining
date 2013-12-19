#include "stdafx.h"
#include "gzip.h"

#include "zlib.h"

#ifdef _MSC_VER
#pragma comment(lib , "zlib.lib")
#endif
namespace gzip
{
bool ungzip(std::vector<unsigned char>& in, size_t count_in, std::vector<unsigned char>& out, size_t& count_out)
{
	int ret,have;
	z_stream d_stream;
	char *compr, *uncompr;
	uLong comprLen, uncomprLen;
	compr = reinterpret_cast<char*>(&in[0]);
	comprLen =count_in;
	out.clear();
	out.resize( count_in * 10,0 );
	uncompr = reinterpret_cast<char*>(&out[0]);
	uncomprLen = out.size();
	memset(&d_stream,0,sizeof(z_stream));
	d_stream.zalloc = Z_NULL;
	d_stream.zfree = Z_NULL;
	d_stream.opaque = Z_NULL;

	d_stream.next_in = Z_NULL;//inflateInit和inflateInit2都必须初始化next_in和avail_in
	d_stream.avail_in = 0;//deflateInit和deflateInit2则不用

	ret = inflateInit2(&d_stream,47);
	if(ret!=Z_OK)
	{
		printf("inflateInit2 error:%d",ret);
		return false;
	}
	count_out = 0;
	d_stream.next_in=reinterpret_cast<Byte*>(compr);
	d_stream.avail_in=comprLen;
	do
	{
		d_stream.next_out=reinterpret_cast<Byte*>(uncompr+count_out);
		d_stream.avail_out=uncomprLen;
		ret = inflate(&d_stream,Z_NO_FLUSH);
		_ASSERT(ret != Z_STREAM_ERROR);
		switch (ret)
		{
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;   
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&d_stream);
			return false;
		}
		have=uncomprLen-d_stream.avail_out;
		
		count_out+=have;

	}while(d_stream.avail_out==0);
	inflateEnd(&d_stream);

	out.erase( out.begin() + count_out, out.end() );
	return true;
}

}