#include "Logger.h"

#include <iostream>

BRWL_NS


Logger::DefaultLogHandler Logger::defaultLogHandler(&BRWL_STD_COUT);

ILogHandler& ILogHandler::operator<<(decltype(::std::endl<BRWL_CHAR, std::char_traits<BRWL_CHAR>>) msg)
{
	operator<<(BRWL_NEWLINE);
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


Logger::ScopedMultiLog::ScopedMultiLog(Logger* log, Logger::LogLevel level) :
	l(log),
	lvl(level)
{
	l->logMutex.lock();
	BRWL_EXCEPTION(l->activeMultiLog == nullptr, BRWL_CHAR_LITERAL("Invalid logger state."));
	l->activeMultiLog = this;
	*l->outStream << Logger::logLevelNames[ENUM_CLASS_TO_NUM(level)] << BRWL_CHAR_LITERAL(": ");
}

Logger::ScopedMultiLog::~ScopedMultiLog()
{
	*l->outStream << std::endl;
	l->activeMultiLog = nullptr;
	l->logMutex.unlock();
}


Logger::Logger() :
	outStream(&Logger::defaultLogHandler),
	activeMultiLog(nullptr)
{ }

Logger::~Logger()
{
	BRWL_EXCEPTION(outStream != nullptr, BRWL_CHAR_LITERAL("Invalid logger state!"));
	outStream->flush();
}

void Logger::debug(const BRWL_CHAR* msg, ScopedMultiLog* multiLog /*= nullptr*/) const
{
	log(msg, LogLevel::DEBUG, multiLog);
}

void Logger::info(const BRWL_CHAR* msg, ScopedMultiLog* multiLog /*= nullptr*/) const
{
	log(msg, LogLevel::INFO, multiLog);
}

void Logger::warning(const BRWL_CHAR* msg, ScopedMultiLog* multiLog /*= nullptr*/) const
{
	log(msg, LogLevel::WARNING, multiLog);
}

void Logger::error(const BRWL_CHAR* msg, ScopedMultiLog* multiLog /*= nullptr*/) const
{
	log(msg, LogLevel::ERROR, multiLog);
}

void Logger::log(const BRWL_CHAR* msg, LogLevel level, ScopedMultiLog* multiLog /*= nullptr*/) const
{
	if (level >= activeLogLevel && outStream != nullptr)
	{
		if (!logMutex.try_lock() && multiLog == activeMultiLog)
		{
			BRWL_CHECK(activeMultiLog->lvl == level, BRWL_CHAR_LITERAL("Log with different log levels called during multi log."));
			*outStream << msg;
		}
		else 
		{
			*outStream << logLevelNames[ENUM_CLASS_TO_NUM(level)] << BRWL_CHAR_LITERAL(": ") << msg << std::endl;
			logMutex.unlock();
		}
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
