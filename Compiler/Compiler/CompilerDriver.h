#pragma once
#include <string>
#include <ostream>
#include "CodegenContext.h"

class CompilerDriver
{
public:
	explicit CompilerDriver(std::ostream& log);

	void Compile(const std::string& text);

	void SaveObjectCodeToFile(const std::string& filepath);
	void SaveIRToFile(const std::string& filepath);

private:
	std::ostream& m_log;
	CodegenContext m_context;
};
