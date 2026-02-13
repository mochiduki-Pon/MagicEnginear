#pragma once
#include "../system/commontypes.h"
#include "../system/Collision.h"

namespace BoxCreate
{
    using AABB = Call::Collision::BoundingBoxAABB;

    struct LedgeBoxLite
    {
        bool enabled = true;
        BoxCreate::AABB box;
    };

    // Z•ûŒü‚ÉAABBBOX‚ğ•À‚×‚é
    struct StripZ
    {
        bool enabled = true;

        float xCenter = 0.0f;
        float zStart = 0.0f;
        float zEnd = 0.0f;
        float pitchZ = 1.0f;

        float halfW = 0.0f;
        float halfLenZ = 0.0f;

        float topY = 0.0f;
    };

    // AABB¶¬ ‚‚³minY/maxYw’è
    inline AABB MakeAabbXZ(
        float xCenter, float zCenter,
        float halfW, float halfLenZ,
        float minY, float maxY)
    {
        AABB aabb;
        aabb.min = Vector3(xCenter - halfW, minY, zCenter - halfLenZ);
        aabb.max = Vector3(xCenter + halfW, maxY, zCenter + halfLenZ);
        return aabb;
    }

    // ‚‚³-1e6f`maxY(’ê‚È‚µ)
    inline void AddBoxXZ(
        std::vector<AABB>& out,
        float xCenter, float zCenter,
        float halfW, float halfLenZ,
        float minY, float maxY)
    {
        out.push_back(
            MakeAabbXZ(xCenter, zCenter, halfW, halfLenZ, minY, maxY)
        );
    }

    // vector<AABB>’ê‚Í‚È‚¢” ¶¬
    void AddBoxXZ(
        std::vector<AABB>& out,
        float xC, float zC,
        float halfW, float halfLenZ,
        float topY);

    // StripZ‚É]‚Á‚ÄAABB‚ğ•À‚×‚Ä out ‚ÉÏ‚Ş
    void BuildStripZ(
        std::vector<AABB>& out,
        const StripZ& s);
}