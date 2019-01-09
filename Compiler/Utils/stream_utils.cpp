#include "stdafx.h"
#include "stream_utils.h"
#include <sstream>

std::string stream_utils::GetStreamContent(std::istream& is)
{
	std::stringstream strm;
	strm << is.rdbuf();
	return strm.str();
}
