#pragma once

#include <chrono>

class Time
{
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;
	using Ms = std::chrono::milliseconds;

	static Time& Get()
	{
		static Time instance;
		return instance;
	}

	TimePoint Now() const
	{
		return Clock::now();
	}

	// floatCast‚ÅŒo‰ßŽžŠÔ‚ðŽæ“¾
	static uint64_t ElapsedMs(TimePoint start, TimePoint now = Clock::now())
	{
		return (uint64_t)std::chrono::duration_cast<Ms>(now - start).count();
	}

private:
	Time() = default;
};