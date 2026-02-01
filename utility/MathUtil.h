#pragma once
#include <cstdint>
#include <random>
#include "../system/commontypes.h"

namespace timeutil
{
    inline void SubTime(uint64_t& value, uint64_t sub) noexcept
    {
        value = (sub >= value) ? 0ULL : (value - sub);
    }
}

namespace Math {
    inline float Distance(const Vector3& a, const Vector3& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
}

namespace randutil
{
    inline std::mt19937& Rng()
    {
        static std::mt19937 rng{ std::random_device{}() };
        return rng;
    }

    inline float RandRange(float min, float max)
    {
        std::uniform_real_distribution<float> d(min, max);
        return d(Rng());
    }
}