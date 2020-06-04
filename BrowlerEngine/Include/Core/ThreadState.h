#pragma once // (c) 2020 Lukas Brunner

BRWL_NS


enum class ThreadState : uint8_t {
    READY = 0,
    RUNNING,
    TERMINATED,
    REWINDING,
};


BRWL_NS_END