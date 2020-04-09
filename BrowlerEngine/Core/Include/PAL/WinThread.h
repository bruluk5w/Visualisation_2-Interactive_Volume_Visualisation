#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

#include "PAL/BrowlerWindowsInclude.h"

#include <functional>
#include <atomic>

#include "ThreadState.h"

BRWL_PAL_NS


template<class returnType>
class WinThread
{
public:
    using ReturnType = returnType;
    using ThreadTarget = std::function<returnType(void)>;

private:
    struct Payload {
        ThreadTarget threadTarget;
        ReturnType* returnValue;
    };


    static DWORD WINAPI ThreadRun(void* param)
    {
        Payload* payload = (Payload*)param;
        if constexpr (std::is_void_v<ReturnType>)
        {
            payload->threadTarget();
        }
        else
        {
            *(payload->returnValue) = payload->threadTarget();
        }
        return 0;
    }

public:
    WinThread(ThreadTarget target, ReturnType* returnValue) :
        payload { std::move(target), std::is_void_v<ReturnType> ? nullptr : returnValue },
        threadHandle(NULL),
        threadId(0),
        state(ThreadState::READY)
    { }

    // may only be called from the creating thread
    bool run()
    {
        ThreadState lastState = state.exchange(ThreadState::RUNNING);
        BRWL_EXCEPTION(lastState == ThreadState::READY, "Wrong thread state when running thread!");
        threadHandle = CreateThread(
            NULL,       // default security attributes
            0,          // use default stack size  
            ThreadRun,  // thread function name
            &payload,    // argument to thread function 
            0,          // use default creation flags 
            &threadId   // returns the thread identifier 
        );

        return BRWL_VERIFY(threadHandle != NULL, "Failed to start thread!");
    }

    // may only be called from the creating thread
    void join()
    {
        // Wait until all threads has terminated.
        WaitForSingleObject(threadHandle, INFINITE);
        DWORD exitCode;
        BRWL_EXCEPTION(GetExitCodeThread(threadHandle, &exitCode), "Failed to retrieve thread exit code!");
        BRWL_EXCEPTION(exitCode == 0, "Thread exit code is not 0!");

        CloseHandle(threadHandle);
        ThreadState lastState = state.exchange(ThreadState::TERMINATED);
        BRWL_EXCEPTION(lastState == ThreadState::RUNNING, "Wrong thread state when joining thread!");
    }

    // may only be called from the creating thread
    void rewind()
    {
        ThreadState lastState = state.exchange(ThreadState::REWINDING);
        BRWL_EXCEPTION(lastState == ThreadState::TERMINATED, "Wrong thread state when rewinding thread!");
        threadHandle = NULL;
        threadId = 0;
        lastState = state.exchange(ThreadState::READY);
        BRWL_EXCEPTION(lastState == ThreadState::REWINDING, "Wrong thread state when setting to ready!");
    }

    ThreadState getState() const { return state.load(); }

    //static joinAll(WinThread* threads, unsigned int numThreads) {

    //    WaitForMultipleObjects(numThreads, hThreadArray, TRUE, INFINITE);
    //}

protected:
    Payload payload;
    HANDLE threadHandle;
    DWORD   threadId;
    std::atomic<ThreadState> state;
};


//#include <functional>


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS
