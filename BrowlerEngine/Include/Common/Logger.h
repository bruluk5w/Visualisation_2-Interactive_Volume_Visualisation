#pragma once

namespace std {
	template <class _Elem, class _Traits> class basic_ostream;
	using ostream = basic_ostream<char, char_traits<char>>;
	template <class _Elem, class _Traits>
	basic_ostream<_Elem, _Traits>& _cdecl endl(
		basic_ostream<_Elem, _Traits>& _Ostr);
}

BRWL_NS

class ILogHandler {
public:
	virtual ILogHandler& operator<<(const BRWL_CHAR* msg) = 0;
	virtual void flush() { }
	virtual ILogHandler& operator<<(decltype(::std::endl<BRWL_CHAR, std::char_traits<BRWL_CHAR>>) msg);
};

class Logger
{
	struct DefaultLogHandler : public ILogHandler {
		friend class Logger;
		DefaultLogHandler(std::basic_ostream<BRWL_CHAR, std::char_traits<BRWL_CHAR>>* outStream) :
			outStream(outStream)
		{
			// this can happen because we come here actually during static initialization of the global variable "app"
			//BRWL_EXCEPTION(outStream == nullptr, L"");
		}

		virtual ILogHandler& operator<<(const BRWL_CHAR* msg) override;

		std::basic_ostream<BRWL_CHAR, std::char_traits<BRWL_CHAR>>* outStream;
	};
	static DefaultLogHandler defaultLogHandler;
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
	virtual ~Logger();

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

	void setOutStream(ILogHandler* logHandler);

protected:
	ILogHandler* outStream;
};
BRWL_NS_END