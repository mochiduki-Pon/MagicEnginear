#include "UI.h"
#include "../system/DirectWrite.h"
#include "../system/CSprite.h"
#include "../system/commontypes.h"
#include "../system/EnemyCounter.h"
#include <DirectXMath.h>
#include <string>
#include <iostream>

namespace
{
    using UV4 = std::array<Vector2, 4>;

    constexpr UV4 UV_FULL = {
        Vector2(0,0),
        Vector2(1,0),
        Vector2(0,1),
        Vector2(1,1)
    };
}

namespace
{
    // 調整用
    namespace UiConst
    {
        // Screen / UI layout
        constexpr float ScreenW = 1280.0f;
        constexpr float ScreenH = 720.0f;

        constexpr float UiW = 640.0f;
        constexpr float UiH = 165.0f;

        constexpr float MarginX = 30.0f;
        constexpr float MarginY = 30.0f;

        // HP / Gauge
        constexpr float BarSrcW = 260.0f;   // テクスチャ基準幅
        constexpr float BarSrcH = 22.0f;    // テクスチャ基準高さ
        constexpr float HpBarVisibleMaxW = 230.0f; // 最大幅（clamp）

        constexpr float HpSmoothSpeedPxPerSec = 500.0f;
        constexpr float DangerHpThreshold01 = 0.35f;

        // HP bar offsets
        constexpr float PlateCenterOffX = 420.0f;
        constexpr float PlateCenterOffY = 140.0f;

        constexpr float BarCenterOffX = 420.0f;
        constexpr float BarCenterOffY = 145.0f;

        constexpr float FillOffX = -10.0f;
        constexpr float FillOffY = -5.0f;

        // Shadow
        constexpr float ShadowExtraW = 10.0f;
        constexpr float ShadowExtraH = 10.0f;

        // Face / shot icons
        constexpr float FaceOffX = 570.0f;
        constexpr float FaceOffY = 110.0f;

        constexpr float ShotOffX = -150.0f;
        constexpr float ShotOffY = -60.0f;

        // Element icons
        constexpr float ElemIconOffX = -217.0f;
        constexpr float ElemIconOffY = -30.0f;

        constexpr float HukidashiOffX = -68.0f;
        constexpr float HukidashiOffY = 30.0f;

        // Enemy gauge (top-right)
        constexpr float EnemyGaugeMarginX = 80.0f;
        constexpr float EnemyGaugeMarginY = 30.0f;

        // Clear icon near gauge
        constexpr float ClearIconW = 64.0f;
        constexpr float ClearIconOffX = -10.0f;
        constexpr float ClearIconOffY = -5.0f;

        // Stage numbers
        constexpr float StageNumX = 125.0f;
        constexpr float StageNumY = 37.0f;
        constexpr float DigitPitch = 24.0f;

        // MP numbers
        constexpr float MpNumX = 30.0f;
        constexpr float MpNumY = 650.0f;

        constexpr float MpIconPadX = 10.0f;
        constexpr float MpIconPadY = 10.0f;

        constexpr float MpNumOffX = 30.0f;
        constexpr float DigitShadowOff = 1.0f;

        // MP orb color
        constexpr float MpOrbR = 0.5f;
        constexpr float MpOrbG = 0.8f;
        constexpr float MpOrbB = 1.0f;
        constexpr float MpOrbA = 1.0f;

        // Digit atlas
        constexpr int DigitTexCols = 5;
        constexpr int DigitTexRows = 2;
        constexpr int DigitW = 42;
        constexpr int DigitH = 42;
    }

    //stage表示素材の配置
    constexpr float STAGE_X = 60.0f;
    constexpr float STAGE_Y = 37.0f;
    //stage〇
    constexpr float STAGEP_X = 70.0f;
    constexpr float STAGEP_Y = 10.0f;
}

namespace {
    struct BarGeom
    {
        float w;      // 現在の幅
        float fillX;  // 現在バー中心X
        float fillY;  // 現在バー中心Y

        float maxW;   // MAX幅
        float maxX;   // MAXバー中心X
        float maxY;   // MAXバー中心Y
    };

    inline BarGeom MakeHpBarGeom(
        float barX, float barY,
        float srcW, float wNow, float wMax,
        float offX, float offY)
    {
        BarGeom g{};
        g.w = wNow;
        g.fillY = barY + offY;

        const float fixXNow = barX + (srcW - wNow) * 0.5f;
        g.fillX = fixXNow + offX;

        g.maxW = wMax;
        g.maxY = g.fillY;

        const float fixXMax = barX + (srcW - wMax) * 0.5f;
        g.maxX = fixXMax + offX;

        return g;
    }
}

UI& UI::Get()
{
    static UI inst;
    return inst;
}

void UI::Init()
{
    if (m_inited) return;
    m_inited = true;

    m_hpplate = std::make_unique<CSprite>(
        static_cast<int>(512 * 0.7f),
        static_cast<int>(76 * 0.7f),
        "assets/texture/f1263_6.png");

    m_face = std::make_unique<CSprite>(
        static_cast<int>(300 * 0.4f),
        static_cast<int>(250 * 0.4f),
        "assets/texture/NekoF.png");

    m_fillG = std::make_unique<CSprite>(static_cast<int>(UiConst::BarSrcW), static_cast<int>(UiConst::BarSrcH), "assets/texture/effectff.png");
    m_fillR = std::make_unique<CSprite>(static_cast<int>(UiConst::BarSrcW), static_cast<int>(UiConst::BarSrcH), "assets/texture/effectr.png");
    m_fillB = std::make_unique<CSprite>(static_cast<int>(UiConst::BarSrcW), static_cast<int>(UiConst::BarSrcH), "assets/texture/effect001.png");
    m_fillW = std::make_unique<CSprite>(static_cast<int>(UiConst::BarSrcW), static_cast<int>(UiConst::BarSrcH), "assets/texture/effect111.png");

    m_shotT = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.5f),
        static_cast<int>(256 * 0.5f),
        "assets/texture/SetShot.png");

    m_shotTtrp = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.5f),
        static_cast<int>(256 * 0.5f),
        "assets/texture/torap.png");

    m_shotTsht = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.42f),
        static_cast<int>(256 * 0.42f),
        "assets/texture/shot.png");

    m_clearmin = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.3f),
        static_cast<int>(256 * 0.3f),
        "assets/texture/Clearmin1.png");

    m_resource = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.3f),
        static_cast<int>(256 * 0.3f),
        "assets/texture/orblight_D.png");

    m_hp01 = 1.0f;
    m_hpSmooth = UiConst::BarSrcW;

    m_256 = std::make_unique<CSprite>(
        static_cast<int>(512.0f * 0.4f),
        static_cast<int>(513.0f * 0.4f),
        "assets/texture/f0971_1.png");

    m_stage = std::make_unique<CSprite>(
        static_cast<int>(700.0f * 0.15f),
        static_cast<int>(200.0f * 0.15f),
        "assets/texture/Stage.png");

    m_syabon = std::make_unique<CSprite>(
        static_cast<int>(190.0f * 0.2f),
        static_cast<int>(190.0f * 0.2f),
        "assets/texture/syabon.png");

    m_fireicon = std::make_unique<CSprite>(
        static_cast<int>(250.0f * 0.25f),
        static_cast<int>(100.0f * 0.25f),
        "assets/texture/fire.png");

    m_hukidashi = std::make_unique<CSprite>(
        static_cast<int>(256 * 0.3f),
        static_cast<int>(256 * 0.3f),
        "assets/texture/SetShotE.png");

    // 数字スプライト
    const std::string DIG_TEX = "assets/texture/num2.png";

    static constexpr int mapDigitToCell[10] = {
        9, // 0 は右下
        0, // 1
        1, // 2
        2, // 3
        3, // 4
        4, // 5
        5, // 6
        6, // 7
        7, // 8
        8  // 9
    };

    for (int digit = 0; digit < 10; ++digit)
    {
        int cell = mapDigitToCell[digit];
        int col = cell % UiConst::DigitTexCols;
        int row = cell / UiConst::DigitTexCols;

        float u0 = col / static_cast<float>(UiConst::DigitTexCols);
        float u1 = (col + 1) / static_cast<float>(UiConst::DigitTexCols);
        float v0 = row / static_cast<float>(UiConst::DigitTexRows);
        float v1 = (row + 1) / static_cast<float>(UiConst::DigitTexRows);

        UV4 uv = {
            Vector2(u0, v0), Vector2(u1, v0),
            Vector2(u0, v1), Vector2(u1, v1)
        };

        m_digitSpr[digit] = std::make_unique<CSprite>(UiConst::DigitW, UiConst::DigitH, DIG_TEX, uv);
    }
}

void UI::SetTrapMode(bool isTrap)
{
    m_isTrapMode = isTrap;
}

void UI::SetCurrentBulletNo(BulletGimmick::BulletNo no)
{
    m_currentNo = no;

    m_state.mode = BulletGimmick::GetMode(no);
    m_state.element = BulletGimmick::GetElement(no);
    m_state.spec = BulletGimmick::Spec(no);
}

void UI::Update(uint64_t dtMs, int hp, int hpMax)
{
    if (!m_inited) Init();

    // HP
    float hp01 = (hpMax > 0) ? (float)hp / (float)hpMax : 0.0f;
    hp01 = std::clamp(hp01, 0.0f, 1.0f);

    const float targetW = UiConst::BarSrcW * hp01;

    if (hp <= 0)
    {
        m_hpSmooth = 0.0f;
        m_hp01 = hp01;
        // return; ここでreturnするとWave更新も止まる
    }

    const float step = UiConst::HpSmoothSpeedPxPerSec * (dtMs * 0.001f);

    if (m_hpSmooth < targetW) m_hpSmooth = std::min(m_hpSmooth + step, targetW);
    else if (m_hpSmooth > targetW) m_hpSmooth = std::max(m_hpSmooth - step, targetW);

    m_hp01 = hp01;

    // 進捗ゲージ
    {
        auto& ec = EnemyCounter::GetInstance();
        const int total = ec.GetWaveTotal();
        const int killed = ec.GetWaveKilled();

        if (total <= 0)
        {
            m_enemyGaugeW = 0.0f;      // ResetWave後に必ずゼロ
            m_enemyTotalSeen = 0;      // 最大保持もリセット
        }
        else
        {
            float ratio = (float)killed / (float)total;
            ratio = std::clamp(ratio, 0.0f, 1.0f);
            m_enemyGaugeW = UiConst::BarSrcW * ratio;
            m_enemyTotalSeen = total; // Wave分母に合わせる
        }
    }
}

void UI::Draw()
{
    if (!m_inited) return;

    const float baseX = UiConst::ScreenW - UiConst::UiW - UiConst::MarginX;
    const float baseY = UiConst::ScreenH - UiConst::UiH - UiConst::MarginY;

    const float faceX = baseX + UiConst::FaceOffX;
    const float faceY = baseY + UiConst::FaceOffY;

    const Vector3 S1(1, 1, 1), R0(0, 0, 0);

    // 枠プレート
    if (m_hpplate)
        m_hpplate->Draw(S1, R0, Vector3(baseX + UiConst::PlateCenterOffX, baseY + UiConst::PlateCenterOffY, 0));

    // HPバー
    const float barX = baseX + UiConst::BarCenterOffX;
    const float barY = baseY + UiConst::BarCenterOffY;
    float w = m_hpSmooth;

    if (w < 0) w = 0;
    if (w > UiConst::HpBarVisibleMaxW) w = UiConst::HpBarVisibleMaxW;

    auto g = MakeHpBarGeom(barX, barY, UiConst::BarSrcW, w, UiConst::HpBarVisibleMaxW, UiConst::FillOffX, UiConst::FillOffY);

    const float sxFill = g.w / UiConst::BarSrcW;

    // 影 MAX固定
    if (m_fillB)
    {
        const float shadowSx = (g.maxW + UiConst::ShadowExtraW) / UiConst::BarSrcW;
        const float shadowSy = (UiConst::BarSrcH + UiConst::ShadowExtraH) / UiConst::BarSrcH;

        m_fillB->Draw(Vector3(shadowSx, shadowSy, 1), R0, Vector3(g.maxX, g.maxY, 0));
    }

    // 本体 可変
    CSprite* fill = (m_hp01 > UiConst::DangerHpThreshold01) ? m_fillG.get() : m_fillR.get();
    if (fill)
    {
        fill->Draw(Vector3(sxFill, 1, 1), R0, Vector3(g.fillX, g.fillY, 0));
    }

    // 顔
    if (m_face)
        m_face->Draw(S1, R0, Vector3(faceX, faceY, 0));

    m_256->Draw(S1, R0, Vector3(STAGEP_X, STAGEP_Y, 0.5));

    // ステージ表示
    if (m_stage)
        m_stage->Draw(S1, R0, Vector3(STAGE_X, STAGE_Y, 0));

    // ショット・トラップ
    const Vector3 shotPos(faceX + UiConst::ShotOffX, faceY + UiConst::ShotOffY, 0);

    if (m_shotT) m_shotT->Draw(S1, R0, shotPos);

    if (m_isTrapMode) {
        if (m_shotTtrp) m_shotTtrp->Draw(S1, R0, shotPos);
    }
    else {
        if (m_shotTsht) m_shotTsht->Draw(S1, R0, shotPos);
    }

    // Element
    {
        const Vector3 elemPos(faceX + UiConst::ElemIconOffX, faceY + UiConst::ElemIconOffY, 0.0f);
        const Vector3 hukiPos = shotPos + Vector3(UiConst::HukidashiOffX, UiConst::HukidashiOffY, 0.0f); // + が右
        if (m_hukidashi)
            m_hukidashi->Draw(S1, R0, hukiPos);

        switch (m_state.element)
        {
        case BulletGimmick::Element::Water:
            if (m_syabon)
                m_syabon->Draw(S1, R0, elemPos);
            break;

        case BulletGimmick::Element::Fire:
            if (m_fireicon)
                m_fireicon->Draw(S1, R0, elemPos);
            break;

        case BulletGimmick::Element::Wind:
            // 風アイコン作ったらここ
            break;
        }
    }

    // Enemyゲージ右上 固定 左→右
    constexpr float BAR_H = UiConst::BarSrcH;

    // 右上に置く左端座標を作る
    const float leftX = UiConst::ScreenW - UiConst::EnemyGaugeMarginX - UiConst::HpBarVisibleMaxW; // 左端
    const float centerY = UiConst::EnemyGaugeMarginY + (BAR_H * 0.5f);

    // 背景黒
    const float bgCenterX = leftX + (UiConst::HpBarVisibleMaxW * 0.5f);

    if (m_fillB)
    {
        m_fillB->Draw(
            Vector3(UiConst::HpBarVisibleMaxW / UiConst::BarSrcW, 1, 1),
            R0,
            Vector3(bgCenterX, centerY, 0)
        );
    }

    // 左端固定で伸ばす
    float enemyW = m_enemyGaugeW;
    if (enemyW < 0) enemyW = 0;
    if (enemyW > UiConst::HpBarVisibleMaxW) enemyW = UiConst::HpBarVisibleMaxW;

    const float fillCenterX = leftX + (enemyW * 0.5f);  // 左端 + 幅の半分
    const float sx = enemyW / UiConst::BarSrcW;

    if (m_fillW)
    {
        m_fillW->Draw(
            Vector3(sx, 1, 1),
            R0,
            Vector3(fillCenterX, centerY, 0)
        );
    }

    const float gaugeRightX = leftX + UiConst::HpBarVisibleMaxW;

    // クリアアイコン
    const float iconCenterX = gaugeRightX + (UiConst::ClearIconW * 0.5f) + UiConst::ClearIconOffX;
    const float iconCenterY = centerY + UiConst::ClearIconOffY;

    if (m_clearmin)
    {
        m_clearmin->Draw(
            Vector3(1, 1, 1),
            R0,
            Vector3(iconCenterX, iconCenterY, 0)
        );
    }

    DrawStageIndex(m_stageIndex);
    DrawMp(m_mpUi);
}

// stage数値の位置
void UI::DrawStageIndex(int stageIndex)
{
    const Vector3 S1(1,1,1), R0(0,0,0);

    int v = (stageIndex < 0) ? 0 : stageIndex;

    int digs[10];
    int n = 0;
    do { digs[n++] = v % 10; v /= 10; } while (v > 0);

    for (int i = 0; i < n; ++i)
    {
        int d = digs[n - 1 - i];
        if (m_digitSpr[d])
            m_digitSpr[d]->Draw(S1, R0, Vector3(UiConst::StageNumX + i * UiConst::DigitPitch, UiConst::StageNumY, 0));
    }
}

void UI::DrawMp(int mp)
{
    const Vector3 S1(1, 1, 1), R0(0, 0, 0);
    const Vector3 S_NUM(0.5f, 0.5f, 1.0f);

    int v = (mp < 0) ? 0 : mp;

    int digs[10];
    int n = 0;
    do { digs[n++] = v % 10; v /= 10; } while (v > 0);

    const float rightX = UiConst::MpNumX + (n * UiConst::DigitPitch);
    const float baseX = rightX + UiConst::MpIconPadX;
    const float baseY = UiConst::MpNumY + UiConst::MpIconPadY;

    if (m_resource)
    {
        m_resource->SetColor(Color(UiConst::MpOrbR, UiConst::MpOrbG, UiConst::MpOrbB, UiConst::MpOrbA));
        m_resource->Draw(S1, R0, Vector3(baseX, baseY, 0));
    }

    // 数字（小さく・右へ）
    const float numX = baseX + UiConst::MpNumOffX;
    for (int i = 0; i < n; ++i)
    {
        int d = digs[n - 1 - i];
        if (!m_digitSpr[d]) continue;

        const float x = numX + i * UiConst::DigitPitch;
        const float y = baseY;

        // 影
        m_digitSpr[d]->Draw(
            Vector3(1, 1, 1), R0,
            Vector3(x + UiConst::DigitShadowOff, y + UiConst::DigitShadowOff, 0)
        );

        // 本体
        m_digitSpr[d]->Draw(
            Vector3(1, 1, 1), R0,
            Vector3(x, y, 0)
        );
    }
}

void UI::UpdateEnemyGauge(int killed, int total)
{
    if (total <= 0) total = 1;

    if (total > m_enemyTotalSeen)
        m_enemyTotalSeen = total;

    float t = (float)killed / (float)m_enemyTotalSeen;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    m_enemyGaugeW = UiConst::BarSrcW * t;
}