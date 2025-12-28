#pragma once 
namespace Core {
class CORE_API Timer {
public:
    Timer() : last_time(std::chrono::steady_clock::now()) {}

    float GetDelta() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - last_time;
        last_time = now;
        return static_cast<float>(std::chrono::duration<double>(delta).count());
    }

private:
    std::chrono::steady_clock::time_point last_time;

}
} // namespace Core
