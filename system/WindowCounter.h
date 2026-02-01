#pragma once
#include <chrono>
#include <cstdint>

class WindowCounter
{
public:
    using TimePoint = std::chrono::steady_clock::time_point;

    void Reset() { m_hasLast = false; m_count = 0; }
    void SetWindowMs(uint64_t ms) { m_windowMs = ms; }

    void OnEvent(TimePoint now)
    {
        if (!m_hasLast)
        {
            m_count = 1;
            m_last = now;
            m_hasLast = true;
            return;
        }

        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last).count();
        m_count = (dt <= (long long)m_windowMs) ? (m_count + 1) : 1;
        m_last = now;
    }

    int Count() const { return m_count; }

private:
    uint64_t m_windowMs = 2000; // ms
    int m_count = 0;
    bool m_hasLast = false;
    TimePoint m_last{};
};