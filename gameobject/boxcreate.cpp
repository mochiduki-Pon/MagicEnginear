#include "boxcreate.h"

namespace BoxCreate
{
    void AddBoxXZ(
        std::vector<AABB>& out,
        float xC, float zC,
        float halfW, float halfLenZ,
        float topY)
    {
        AABB b{};
        b.min = Vector3(xC - halfW, -1.0e6f, zC - halfLenZ);
        b.max = Vector3(xC + halfW, topY, zC + halfLenZ);
        out.push_back(b);
    }

    void BuildStripZ(
        std::vector<AABB>& out,
        const StripZ& s)
    {
        if (!s.enabled) return;
        if (s.pitchZ <= 1e-6f) return;

        for (float z = s.zStart; z <= s.zEnd + 0.0001f; z += s.pitchZ)
        {
            AddBoxXZ(out, s.xCenter, z, s.halfW, s.halfLenZ, s.topY);
        }
    }
}