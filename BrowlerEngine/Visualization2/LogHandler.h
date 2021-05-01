#pragma once // (c) 2020 Lukas Brunner

#include "Common/Logger.h"
#include <algorithm>

class LogHandler : public BRWL::ILogHandler {
public:
	LogHandler(HWND target) :
		messageTarget(target),
		stringPool(),
		occupied()
	{
		BRWL_CHECK(target != NULL, BRWL_CHAR_LITERAL("Need a target to send log messages to"));
		stringPool.reserve(50);
		occupied.reserve(50);
		buffer.reserve(500);
	}

	virtual ILogHandler& operator<<(const BRWL_CHAR* msg) override;
	virtual void flush() override;

	const BRWL_STR& getStringRef(LPARAM index) {
		poolMutex.lock();
		return stringPool[index];
	}

	void setStringDone(LPARAM index) {
		occupied[index] = false;
		poolMutex.unlock();
	}

private:
	static std::mutex poolMutex;
	static std::mutex writeMutex;
	HWND messageTarget;
	std::vector<BRWL_STR> stringPool;
	std::vector<bool> occupied;
	BRWL_STR buffer;
};