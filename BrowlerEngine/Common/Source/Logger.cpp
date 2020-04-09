#include "Logger.h"

#include <iostream>


BRWL_NS


const char* Logger::logLevelNames[ENUM_CLASS_TO_NUM(Logger::LogLevel::MAX)] = {
	"  DEBUG",
	"   INFO",
	"WARNING",
	"  ERROR",
};

Logger::Logger() :
	outStream(&std::cout)
{ }

void Logger::debug(const char* msg) const
{
	log(msg, LogLevel::DEBUG);
}

void Logger::info(const char* msg) const
{
	log(msg, LogLevel::INFO);
}

void Logger::warning(const char* msg) const
{
	log(msg, LogLevel::WARNING);
}

void Logger::error(const char* msg) const
{
	log(msg, LogLevel::ERROR);
}

void Logger::log(const char* msg, LogLevel level) const
{
	if (level >= activeLogLevel && outStream != nullptr)
	{
		*Logger::outStream << logLevelNames[ENUM_CLASS_TO_NUM(level)] << ": " << msg << std::endl;
	}
}


BRWL_NS_END