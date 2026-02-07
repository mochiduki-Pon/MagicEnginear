#pragma once
#include <cstdint>
#include <array>
#include "gameobject.h"

class BulletGimmick
{
public:

    //属性
    enum class Element : uint8_t { Water = 0, Fire = 1, Wind = 2 };
	//shot or trap
    enum class BulletMode : uint8_t { Shot = 0, Trap = 1 };
    enum class BulletNo : uint8_t {
        WaterShot = 0, WaterTrap = 1, FireShot = 2, FireTrap = 3, WindShot = 4, WindTrap = 5 };

    struct BulletSpec { float speed, lifeSec, radius; int damage; int mpCost; bool isTrap; float kbH; float kbV; };

    // 見た目取り付け
    struct BulletVisualSpec
    {
        // 0:左上 1:右上 2:左下 3:右下
        Color c0 = Color(1, 1, 1, 0.6f);
        Color c1 = Color(1, 1, 1, 0.6f);
        Color c2 = Color(0, 1, 1, 0.6f);
        Color c3 = Color(0, 0, 1, 0.6f);

        const char* texture = "assets/texture/Circle256.png";

        // Trap
        bool  pulse = false;
        float pulseSpeed = 8.0f;
        float pulseAmp = 0.20f;

        // スケール
        float scale = 1.0f;
    };

	// ElementとBulletNoを合わせる
    static constexpr BulletNo MakeBulletNo(Element e, BulletMode m)
    {
        return static_cast<BulletNo>(static_cast<uint8_t>(e) * 2u + static_cast<uint8_t>(m));
    }

	// 指定の BulletNo の性能仕様を取得
    static constexpr const BulletSpec& Spec(BulletNo no)
    {
        const auto i = static_cast<uint8_t>(no);
        assert(i < std::size(kSpec));
        return kSpec[i];
    }

	// 指定の BulletNo の見た目を取得
    static constexpr const BulletVisualSpec& VSpec(BulletNo no)
    {
        const auto i = static_cast<uint8_t>(no);
        assert(i < kVSpec.size());
        return kVSpec[i];
    }

private:

    uint32_t m_acceptBulletNo = 1;

    inline static constexpr BulletSpec kSpec[6] = {
   //speed life radius dmg mpCost trap   kbH   kbV
    {5.0f, 2.0f,  60.0f, 1, 0, false,  6.0f,  4.0f},  // WaterShot
    {0.0f,90.0f, 200.0f, 2, 10, true,  14.0f, 12.0f},  // WaterTrap
    {8.0f, 3.0f, 20.0f, 1, 1, false,  8.0f,  6.0f},  // FireShot
    {0.0f, 6.0f,160.0f, 5, 10, true,  18.0f, 10.0f},  // FireTrap
    {6.0f, 1.8f, 80.0f, 0, 0, false,  10.0f,  10.0f},  // WindShot
    {0.0f,10.0f,140.0f, 0, 10, true,   0.0f,  0.0f},  // WindTrap
    };

    // 4頂点色
    inline static constexpr std::array<BulletVisualSpec, 6> kVSpec = { {
    //点滅bool,点滅の速さ,点滅の幅%,見た目スケール
    // WaterShot
    { Color(1,1,1,0.6f), Color(1,1,1,0.6f), Color(0,1,1,0.6f), Color(0,0,1,0.6f),
    "assets/texture/Circle256.png", false, 8.0f, 0.2f, 1.0f },

    // WaterTrap
    { Color(1,1,1,0.6f), Color(1,1,1,0.6f), Color(0.3f,1,1,0.6f), Color(0,0.4f,1,0.6f),
    "assets/texture/Circle256.png", true, 8.0f, 0.20f, 1.2f },

    // FireShot
    { Color(1.0f, 1.0f, 0.0f, 0.6f), Color(1,0.5,0,0.6f), Color(1,0.5,0,0.6f), Color(1,0,0,0.6f),
    "assets/texture/Circle256.png", false, 10.0f, 0.25f, 1.0f },

    // FireTrap
    { Color(1,1,1,0.6f), Color(1,1,1,0.6f), Color(1,0.6f,0.2f,0.6f), Color(1,0.2f,0.1f,0.6f),
    "assets/texture/Circle256.png", true, 10.0f, 0.25f, 1.3f },

    // WindShot
    { Color(1,1,1,0.6f), Color(1,1,1,0.6f), Color(0.4f,1,0.4f,0.6f), Color(0.1f,0.8f,0.1f,0.6f),
    "assets/texture/Circle256.png", false, 9.0f, 0.22f, 1.0f },

    // WindTrap
    { Color(1,1,1,0.6f), Color(1,1,1,0.6f), Color(0.4f,1,0.4f,0.6f), Color(0.1f,0.8f,0.1f,0.6f),
    "assets/texture/Circle256.png", true, 9.0f, 0.22f, 1.2f },
    } };

};