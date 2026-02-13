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
    constexpr float SRC_H = 22.0f;

    constexpr float STAGE_X = 70.0f;
    constexpr float STAGE_Y = 50.0f;
    constexpr float STAGEP_X = 50.0f;
    constexpr float STAGEP_Y = -20.0f;
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
        static_cast<int>(300 * 0.5f),
        static_cast<int>(250 * 0.5f),
        "assets/texture/NekoF.png");

    m_fillG = std::make_unique<CSprite>(260, 22, "assets/texture/effectff.png");
    m_fillR = std::make_unique<CSprite>(260, 22, "assets/texture/effectr.png");
    m_fillB = std::make_unique<CSprite>(260, 22, "assets/texture/effect001.png");
    m_fillW = std::make_unique<CSprite>(260, 22, "assets/texture/effect111.png");

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
    m_hpSmooth = 260.0f;

    m_256 = std::make_unique<CSprite>(
        static_cast<int>(512.0f * 0.7f),
        static_cast<int>(513.0f * 0.7f),
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

    constexpr int DIG_W = 42;
    constexpr int DIG_H = 42;


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
        int col = cell % 5;
        int row = cell / 5;

        float u0 = col / 5.0f;
        float u1 = (col + 1) / 5.0f;
        float v0 = row / 2.0f;
        float v1 = (row + 1) / 2.0f;

        UV4 uv = {
            Vector2(u0, v0), Vector2(u1, v0),
            Vector2(u0, v1), Vector2(u1, v1)
        };

        m_digitSpr[digit] = std::make_unique<CSprite>(DIG_W, DIG_H, DIG_TEX, uv);
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

    constexpr float barW = 260.0f;

    // HP
    float hp01 = (hpMax > 0) ? (float)hp / (float)hpMax : 0.0f;
    hp01 = std::clamp(hp01, 0.0f, 1.0f);

    const float targetW = barW * hp01;

    if (hp <= 0)
    {
        m_hpSmooth = 0.0f;
        m_hp01 = hp01;
        // return; ここでreturnするとWave更新も止まる
    }

    const float speed = 500.0f;
    const float step = speed * (dtMs * 0.001f);

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
            m_enemyGaugeW = 260.0f * ratio;
            m_enemyTotalSeen = total; // Wave分母に合わせる
        }
    }
}

void UI::Draw()
{
    if (!m_inited) return;

    constexpr float SCREEN_W = 1280.0f;
    constexpr float SCREEN_H = 720.0f;

    constexpr float UI_W = 640.0f;
    constexpr float UI_H = 165.0f;

    constexpr float MARGIN_X = 30.0f;
    constexpr float MARGIN_Y = 30.0f;

    const float baseX = SCREEN_W - UI_W - MARGIN_X;
    const float baseY = SCREEN_H - UI_H - MARGIN_Y;

    constexpr float SRC_W = 260.0f;
    constexpr float VISIBLE_W = 220.0f;

    constexpr float BG_OFF_X = -9.0f;
    constexpr float BG_OFF_Y = 0.0f;
    constexpr float FILL_OFF_X = -10.0f;
    constexpr float FILL_OFF_Y = -5.0f;

    const float faceX = baseX + 570.0f;
    const float faceY = baseY + 110.0f;

    const Vector3 S1(1, 1, 1), R0(0, 0, 0);

    // 枠プレート
    if (m_hpplate)
        m_hpplate->Draw(S1, R0, Vector3(baseX + 420.0f, baseY + 140.0f, 0));

    // HPバー
    const float barX = baseX + 420.0f;
    const float barY = baseY + 145.0f;
    float w = m_hpSmooth;

    if (w < 0) w = 0;
    if (w > 230.0f) w = 230.0f;

    constexpr float W_MAX = 230.0f;

    auto g = MakeHpBarGeom(barX, barY, SRC_W, w, W_MAX, FILL_OFF_X, FILL_OFF_Y);

    const float sxFill = g.w / SRC_W;

    // 影 MAX固定
    constexpr float SHADOW_EXTRA_W = 10.0f;
    constexpr float SHADOW_EXTRA_H = 10.0f;

    if (m_fillB)
    {
        const float shadowSx = (g.maxW + SHADOW_EXTRA_W) / SRC_W;
        const float shadowSy = (SRC_H + SHADOW_EXTRA_H) / SRC_H;

        m_fillB->Draw(Vector3(shadowSx, shadowSy, 1), R0, Vector3(g.maxX, g.maxY, 0));
    }

    // 本体 可変
    CSprite* fill = (m_hp01 > 0.35f) ? m_fillG.get() : m_fillR.get();
    if (fill)
    {
        fill->Draw(Vector3(sxFill, 1, 1), R0, Vector3(g.fillX, g.fillY, 0));
    }

    // 顔
    if (m_face)
        m_face->Draw(S1, R0, Vector3(baseX + 570.0f, baseY + 110.0f, 0));

    m_256->Draw(S1, R0, Vector3(STAGEP_X, STAGEP_Y, 0.5));

	// ステージ表示
    if (m_stage)
        m_stage->Draw(S1, R0, Vector3(STAGE_X, STAGE_Y, 0));

	// ショット・トラップ
    const Vector3 shotPos(faceX - 150.0f, faceY - 60.0f, 0);

    if (m_shotT) m_shotT->Draw(S1, R0, shotPos);

    if (m_isTrapMode) {
        if (m_shotTtrp) m_shotTtrp->Draw(S1, R0, shotPos);
    }
    else {
        if (m_shotTsht) m_shotTsht->Draw(S1, R0, shotPos);
    }

    // Element
    {
        constexpr float ICON_OX = -217.0f;
        constexpr float ICON_OY = -30.0f;

        const Vector3 elemPos(faceX + ICON_OX, faceY + ICON_OY, 0.0f);
        const Vector3 hukiPos = shotPos + Vector3(-68.0f, 30.0f, 0.0f); // + が右
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
    constexpr float TOP_MX = 80.0f;
    constexpr float TOP_MY = 30.0f;

    constexpr float BAR_W = 260.0f;
    constexpr float BAR_H = SRC_H;   // 22

    // 右上に置く左端座標を作る
    const float leftX = SCREEN_W - TOP_MX - W_MAX;      // 左端
    const float centerY = TOP_MY + (BAR_H * 0.5f);

    // 背景黒
    const float bgCenterX = leftX + (W_MAX * 0.5f);

    if (m_fillB)
    {
        m_fillB->Draw(
            Vector3(W_MAX / SRC_W, 1, 1),
            R0,
            Vector3(bgCenterX, centerY, 0)
        );
    }

    // 左端固定で伸ばす
    float enemyW = m_enemyGaugeW;
    if (enemyW < 0) enemyW = 0;
    if (enemyW > W_MAX) enemyW = W_MAX;

    const float fillCenterX = leftX + (enemyW * 0.5f);  // 左端 + 幅の半分
    const float sx = enemyW / SRC_W;

    if (m_fillW)
    {
        m_fillW->Draw(
            Vector3(sx, 1, 1),
            R0,
            Vector3(fillCenterX, centerY, 0)
        );
    }

    const float gaugeRightX = leftX + W_MAX;

    // クリアアイコン
    constexpr float ICON_W = 64.0f;
    constexpr float ICON_OFF_X = -10.0f;
    constexpr float ICON_OFF_Y = -5.0f;

    const float iconCenterX = gaugeRightX + (ICON_W * 0.5f) + ICON_OFF_X;
    const float iconCenterY = centerY + ICON_OFF_Y;

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

void UI::DrawStageIndex(int stageIndex)
{
    const Vector3 S1(1,1,1), R0(0,0,0);

    constexpr float NUM_X = 138.0f; // STAGE画像右に合わせて調整
    constexpr float NUM_Y = 50.0f;
    constexpr float PITCH = 24.0f; // 桁間

    int v = (stageIndex < 0) ? 0 : stageIndex;

    int digs[10];
    int n = 0;
    do { digs[n++] = v % 10; v /= 10; } while (v > 0);

    for (int i = 0; i < n; ++i)
    {
        int d = digs[n - 1 - i];
        if (m_digitSpr[d])
            m_digitSpr[d]->Draw(S1, R0, Vector3(NUM_X + i * PITCH, NUM_Y, 0));
    }
}

void UI::DrawMp(int mp)
{
    const Vector3 S1(1, 1, 1), R0(0, 0, 0);
    const Vector3 S_NUM(0.5f, 0.5f, 1.0f);

    constexpr float NUM_X = 30.0f;
    constexpr float NUM_Y = 650.0f;
    constexpr float PITCH = 24.0f;

    int v = (mp < 0) ? 0 : mp;

    int digs[10];
    int n = 0;
    do { digs[n++] = v % 10; v /= 10; } while (v > 0);

    const float rightX = NUM_X + (n * PITCH);
    const float baseX = rightX + 10.0f;
    const float baseY = NUM_Y + 10.0f;

    // iyadaaaaaaa
    if (m_resource)
    {
        m_resource->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        m_resource->Draw(S1, R0, Vector3(baseX, baseY, 0));
    }

    // 数字（小さく・右へ）
    const float numX = baseX + 30.0f;
    for (int i = 0; i < n; ++i)
    {
        int d = digs[n - 1 - i];
        if (!m_digitSpr[d]) continue;

        const float x = numX + i * PITCH;
        const float y = baseY;

        // 影（先に）
        m_digitSpr[d]->Draw(
            Vector3(1, 1, 1), R0,
            Vector3(x + 1, y + 1, 0)
        );

        // 本体（後に）
        m_digitSpr[d]->Draw(
            Vector3(1, 1, 1), R0,
            Vector3(x, y, 0)
        );
    }
}

void UI::UpdateEnemyGauge(int killed, int total)
{
    constexpr float BAR_W = 260.0f;
    if (total <= 0) total = 1;

    if (total > m_enemyTotalSeen)
        m_enemyTotalSeen = total;

    float t = (float)killed / (float)m_enemyTotalSeen;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    m_enemyGaugeW = BAR_W * t;
}