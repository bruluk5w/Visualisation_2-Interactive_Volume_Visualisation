#pragma once
#include "pch.h"
#include "Common/Logger.h"
#include <vector>
#include <algorithm>
#include <mutex>

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
	}

	virtual ILogHandler& operator<<(const BRWL_CHAR* msg) override
	{
		size_t index = 0;
		{
			std::scoped_lock(mutex);
			// TODO: use 3 arrays of fixed sizes: 1 for the pool of strings, 1 for pointers to free strings, 1 for pointers to occupied strings
			const auto found = std::find(occupied.begin(), occupied.end(), false);
			index = found - occupied.begin();
			if (found == occupied.end())
			{
				stringPool.emplace_back(msg);
				occupied.emplace_back(true);
			}
			else
			{
				stringPool[index].clear();
				stringPool[index] = msg;
				occupied[index] = true;
			}
		}

		::PostMessage(messageTarget, WM_USER_POST_LOG_MSG, 0, index);
		return *this;
	}

	const BRWL_STR& getStringRef(LPARAM index) {
		mutex.lock();
		return stringPool[index];
	}

	void setStringDone(LPARAM index) {
		occupied[index] = false;
		mutex.unlock();
	}

private:
	static std::mutex mutex;
	HWND messageTarget;
	std::vector<BRWL_STR> stringPool;
	std::vector<bool> occupied;

};