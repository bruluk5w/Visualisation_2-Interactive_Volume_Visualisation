#pragma once

namespace std {
	template <class _Elem, class _Traits> class basic_ostream;
	using ostream = basic_ostream<char, char_traits<char>>;
}

BRWL_NS

class Logger
{
public:
	Logger() : outStream(nullptr) { }

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

	static const BRWL_CHAR* logLevelNames[ENUM_CLASS_TO_NUM(LogLevel::MAX)];
#ifdef _DEBUG
	static const LogLevel activeLogLevel = Logger::LogLevel::DEBUG;
#else
	static const LogLevel activeLogLevel = Logger::LogLevel::INFO;
#endif


	void debug(const BRWL_CHAR* msg) const;
	void info(const BRWL_CHAR* msg) const;
	void warning(const BRWL_CHAR* msg) const;
	void error(const BRWL_CHAR* msg) const;
	void log(const BRWL_CHAR* msg, LogLevel level) const;

	void setOutStream(std::ostream* loggingSink) { BRWL_EXCEPTION(outStream, BRWL_CHAR_LITERAL("Stream already set!")); outStream = loggingSink; }

protected:
	std::ostream* outStream;
};
BRWL_NS_END