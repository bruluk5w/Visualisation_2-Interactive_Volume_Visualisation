#include "LogHandler.h"


std::mutex LogHandler::poolMutex = std::mutex();

BRWL::ILogHandler& LogHandler::operator<<(const BRWL_CHAR* msg)
{
	std::scoped_lock(writeMutex);
	buffer.append(msg);
	return *this;
}

void LogHandler::flush()
{
	size_t index = 0;
	std::scoped_lock(poolMutex);
	// TODO: use 3 arrays of fixed sizes: 1 for the pool of strings, 1 for pointers to free strings, 1 for pointers to occupied strings
	const auto found = std::find(occupied.begin(), occupied.end(), false);
	index = found - occupied.begin();
	if (found == occupied.end())
	{
		{
			std::scoped_lock(writeMutex);
			stringPool.emplace_back(buffer);
			buffer.clear();
		}

		occupied.emplace_back(true);
	}
	else
	{
		stringPool[index].clear();
		{
			std::scoped_lock(writeMutex);
			stringPool[index] = buffer;
			buffer.clear();
		}

		occupied[index] = true;
	}

	::PostMessage(messageTarget, WM_USER_POST_LOG_MSG, 0, index);

}
