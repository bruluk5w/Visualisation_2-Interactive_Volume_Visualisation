#include "Logger.h"

#include <iostream>


BRWL_NS


const BRWL_CHAR* Logger::logLevelNames[ENUM_CLASS_TO_NUM(Logger::LogLevel::MAX)] = {
	BRWL_CHAR_LITERAL("  DEBUG"),
	BRWL_CHAR_LITERAL("   INFO"),
	BRWL_CHAR_LITERAL("WARNING"),
	BRWL_CHAR_LITERAL("  ERROR"),
};

Logger::Logger() :
	outStream(&std::cout)
{ }

void Logger::debug(const BRWL_CHAR* msg) const
{
	log(msg, LogLevel::DEBUG);
}

void Logger::info(const BRWL_CHAR* msg) const
{
	log(msg, LogLevel::INFO);
}

void Logger::warning(const BRWL_CHAR* msg) const
{
	log(msg, LogLevel::WARNING);
}

void Logger::error(const BRWL_CHAR* msg) const
{
	log(msg, LogLevel::ERROR);
}

void Logger::log(const BRWL_CHAR* msg, LogLevel level) const
{
	if (level >= activeLogLevel && outStream != nullptr)
	{
		*Logger::outStream << logLevelNames[ENUM_CLASS_TO_NUM(level)] << ": " << msg << std::endl;
	}
}


BRWL_NS_END