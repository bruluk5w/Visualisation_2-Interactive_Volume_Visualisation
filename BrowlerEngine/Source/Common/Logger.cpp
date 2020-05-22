#include "Logger.h"

#include <iostream>

BRWL_NS


Logger::DefaultLogHandler Logger::defaultLogHandler(&BRWL_STD_COUT);

ILogHandler& ILogHandler::operator<<(decltype(::std::endl<BRWL_CHAR, std::char_traits<BRWL_CHAR>>) msg)
{
	operator<<(BRWL_CHAR_LITERAL("\r\n"));
	flush();
	return *this;
}

ILogHandler& Logger::DefaultLogHandler::operator<<(const BRWL_CHAR* msg)
{
	if (outStream) *outStream << msg;
	return *this;
}

const BRWL_CHAR* Logger::logLevelNames[ENUM_CLASS_TO_NUM(Logger::LogLevel::MAX)] = {
	BRWL_CHAR_LITERAL("  DEBUG"),
	BRWL_CHAR_LITERAL("   INFO"),
	BRWL_CHAR_LITERAL("WARNING"),
	BRWL_CHAR_LITERAL("  ERROR"),
};

Logger::Logger() : outStream(&Logger::defaultLogHandler)
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
		*Logger::outStream << logLevelNames[ENUM_CLASS_TO_NUM(level)] << BRWL_CHAR_LITERAL(": ") << msg << std::endl;
	}
}

void Logger::setOutStream(ILogHandler* logHandler)
{
	if(logHandler != nullptr)
	{
		outStream = logHandler;
	}
	else {
		outStream = &defaultLogHandler;
	}

	*outStream << BRWL_CHAR_LITERAL("Out stream of logger changed! Logging Level: ") << logLevelNames[ENUM_CLASS_TO_NUM(activeLogLevel)] << std::endl;
}


BRWL_NS_END
