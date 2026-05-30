#include "Timer.h"
namespace Core {
    Timer::Timer() : last_time(std::chrono::steady_clock::now()) {}

    float Timer::GetDelta() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - last_time;
        last_time = now;
        return static_cast<float>(std::chrono::duration<double>(delta).count());
    }

    Timer& Timer::GetGlobalInstance() {
        static Timer instance;
        return instance;
    }
}