#pragma once
#include <memory>
#include <string>
#include <fstream>

namespace FileUtils
{
std::unique_ptr<std::ifstream> OpenFileForReading(
	const std::string& filepath, std::ios::openmode mode = std::ios::in);

std::unique_ptr<std::ofstream> OpenFileForWriting(
	const std::string& filepath, std::ios::openmode mode = std::ios::out);
}
