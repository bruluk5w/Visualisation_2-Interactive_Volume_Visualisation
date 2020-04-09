#pragma once

namespace std {
	template <class _Elem, class _Traits> class basic_ostream;
	using ostream = basic_ostream<char, char_traits<char>>;
}

BRWL_NS

class Logger
{
public:
	enum class LogLevel : uint8_t
	{
		DEBUG = 0,
		INFO,
		WARNING,
		ERROR,

		MAX,
		MIN = DEBUG
	};

	Logger();

	static const char* logLevelNames[ENUM_CLASS_TO_NUM(LogLevel::MAX)];
#ifdef _DEBUG
	static const LogLevel activeLogLevel = Logger::LogLevel::DEBUG;
#else
	static const LogLevel activeLogLevel = Logger::LogLevel::INFO;
#endif


	void debug(const char* msg) const;
	void info(const char* msg) const;
	void warning(const char* msg) const;
	void error(const char* msg) const;
	void log(const char* msg, LogLevel level) const;

	void setOutStream(std::ostream* loggingSink) { outStream = loggingSink; }

protected:
	std::ostream* outStream;
};
BRWL_NS_END