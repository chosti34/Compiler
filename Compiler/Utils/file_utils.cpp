#include "stdafx.h"
#include "file_utils.h"
#include "stream_utils.h"

namespace file_utils
{
std::unique_ptr<std::ifstream> OpenFileForReading(const std::string& filepath, std::ios::openmode mode)
{
	auto file = std::make_unique<std::ifstream>(filepath, mode);
	if (!file->is_open())
	{
		throw std::runtime_error("failed to open '" + filepath + "' for reading");
	}
	return file;
}

std::unique_ptr<std::ofstream> OpenFileForWriting(const std::string& filepath, std::ios::openmode mode)
{
	auto file = std::make_unique<std::ofstream>(filepath, mode);
	if (!file->is_open())
	{
		throw std::runtime_error("failed to open '" + filepath + "' for writing");
	}
	return file;
}

std::string GetFileContent(const std::string& filepath)
{
	auto file = file_utils::OpenFileForReading(filepath);
	return stream_utils::GetStreamContent(*file);
}
}
