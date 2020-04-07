#pragma once

BRWL_NS


enum class ThreadState : uint8_t {
    READY = 0,
    RUNNING,
    TERMINATED,
    REWINDING,
};


BRWL_NS_END