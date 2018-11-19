#ifndef COMPILER_60MIN_FILEUTIL_H
#define COMPILER_60MIN_FILEUTIL_H

#include <fstream>
#include <string>

namespace file_utils
{
std::unique_ptr<std::ifstream> OpenFileForReading(const std::string& filepath, std::ios::openmode mode = std::ios::in);
std::unique_ptr<std::ofstream> OpenFileForWriting(const std::string& filepath, std::ios::openmode mode = std::ios::out);
}

#endif //COMPILER_60MIN_FILEUTIL_H
