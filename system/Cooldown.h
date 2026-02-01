#pragma once

#include "Time.h"

class Cooldown
{
public:
	void SetInterval(Time::Ms interval)
	{
		m_interval = interval;
	}

	void Reset()
	{
		m_next = Time::TimePoint{};
	}

	bool Consume(Time::TimePoint now)
	{
		if (now < m_next)
		{
			return false;
		}

		m_next = now + m_interval;
		return true;
	}

	// 追加：呼ぶだけ版オーバーロード
	bool Consume()
	{
		return Consume(Time::Get().Now());
	}

private:
	Time::TimePoint m_next{};
	Time::Ms m_interval{ 0 };
};