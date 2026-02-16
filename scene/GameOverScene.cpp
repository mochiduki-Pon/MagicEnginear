#include "GameOverScene.h"
#include "../system/CDirectInput.h"
#include "../system/scenemanager.h"
#include "../system/renderer.h"
#include "../scene/TitleScene.h"
#include "../system/EnemyCounter.h"
#include "../sound.h"
#include <iostream>
#include "../gameobject/UI.h"
#include <xinput.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#pragma comment(lib, "xinput.lib")

namespace
{
    inline float NormStick(SHORT v)
    {
        float f = (v < 0) ? (float)v / 32768.0f : (float)v / 32767.0f;
        return std::clamp(f, -1.0f, 1.0f);
    }

    inline void RadialDeadZone(float& x, float& y, float dz)
    {
        const float len2 = x * x + y * y;
        if (len2 < dz * dz) { x = 0.0f; y = 0.0f; return; }

        const float len = std::sqrt(len2);
        if (len <= 1e-6f) { x = 0.0f; y = 0.0f; return; }

        const float s = std::clamp((len - dz) / (1.0f - dz), 0.0f, 1.0f);
        x = (x / len) * s;
        y = (y / len) * s;
    }

    struct PadState
    {
        bool connected{};
        float lx{};   // -1..1
        float ly{};   // -1..1 (up positive)
        bool a{};     // hold
        bool aTrg{};  // trigger
        bool upTrg{};
        bool downTrg{};
    };

    inline PadState ReadPad()
    {
        PadState p{};

        XINPUT_STATE st{};
        std::memset(&st, 0, sizeof(st));
        if (XInputGetState(0, &st) != ERROR_SUCCESS)
            return p;

        p.connected = true;

        p.lx = NormStick(st.Gamepad.sThumbLX);
        p.ly = NormStick(st.Gamepad.sThumbLY);
        RadialDeadZone(p.lx, p.ly, 0.25f);

        const bool nowA = (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;

        // 簡易トリガ
        static bool prevA = false;
        p.a = nowA;
        p.aTrg = (nowA && !prevA);
        prevA = nowA;

        // メニュー移動へ変換
        constexpr float TH = 0.60f;
        const bool nowUp = (p.ly >= TH);
        const bool nowDown = (p.ly <= -TH);

        static bool prevUp = false;
        static bool prevDown = false;

        p.upTrg = (nowUp && !prevUp);
        p.downTrg = (nowDown && !prevDown);

        prevUp = nowUp;
        prevDown = nowDown;

        return p;
    }
}

GameOverScene::GameOverScene() {}

void GameOverScene::init()
{
    DirectWrite::GetInstance().Init(Renderer::GetSwapChain());

    m_font.fontSize = 32;
    m_font.Color = D2D1::ColorF(0.30f, 0.22f, 0.16f, 1.0f);
    DirectWrite::GetInstance().SetFont(m_font);

    static const std::array<Vector2, 4> UV_FULL = {
        Vector2(0, 0), Vector2(1, 0),
        Vector2(0, 1), Vector2(1, 1)
    };

    // ボタンプレート
    m_plate[0] = std::make_unique<CSprite>(
        static_cast<int>(512.0f * 0.5f),
        static_cast<int>(120.0f * 0.5f),
        "assets/texture/f1408_1.png",
        UV_FULL
    );
    m_plate[1] = std::make_unique<CSprite>(
        static_cast<int>(512.0f * 0.5f),
        static_cast<int>(120.0f * 0.5f),
        "assets/texture/f1408_1.png",
        UV_FULL
    );

    // カーソル
    m_cursor = std::make_unique<CSprite>(50, 50, "assets/texture/914610.png", UV_FULL);
    m_accept = std::make_unique<CSprite>(
        static_cast<int>(390.0f * 0.4f),
        static_cast<int>(228.0f * 0.4f),
        "assets/texture/Abotan.png",
        UV_FULL
    );

    m_over = std::make_unique<CSprite>(300, 300, "assets/texture/over.png", UV_FULL);
    m_bg = std::make_unique<CSprite>(
        Application::GetWidth(),
        Application::GetHeight(),
        "assets/texture/cEND.png",
        UV_FULL
    );

    m_circle = std::make_unique<CSprite>(256, 256, "assets/texture/Circle256_o.png", UV_FULL);

    // 状態初期化
    m_select = 0;
    m_decided = false;
    m_enterFade = true;
    m_circleScale = START_SCALE;
    m_fadeStart = Time::Get().Now();

    // ★追加：ふよふよ開始時刻
    m_bobStart = Time::Get().Now();

    GetXAud()->soundSEPlay((int)SoundSEAssets::cEND, -0.5f);
}

// 行中央Y座標
float GameOverScene::RowCenterY(int idx, float cy) const
{
    static constexpr float BASE_Y_OFFSET = -50.0f;
    const float y0 = cy + BASE_Y_OFFSET;
    return y0 + idx * ROW_SPACING;
}

// プレート内文字矩形
D2D1_RECT_F GameOverScene::PlateTextRect(float cx, float rowCY) const
{
    const float left = (cx - PLATE_W * 0.5f) + TEXT_PAD_L;
    const float right = (cx + PLATE_W * 0.5f) - TEXT_PAD_R;
    return D2D1_RECT_F{ left, rowCY - TEXT_HALF_H, right, rowCY + TEXT_HALF_H };
}

// カーソルX座標
float GameOverScene::CursorX(float cx) const
{
    return (cx - PLATE_W * 0.5f) + CURSOR_X_INSET;
}

// メニュー文字描画
void GameOverScene::DrawMenuText(const char* text, float cx, float rowCY)
{
    DirectWrite::GetInstance().DrawString(
        std::string(text),
        PlateTextRect(cx, rowCY),
        D2D1_DRAW_TEXT_OPTIONS_NONE,
        false
    );
}

void GameOverScene::update(uint64_t)
{
    static constexpr float FADE_MS = 800.0f;      // フェード時間
    static constexpr float START_SCALE = 8.0f;    // 円の最大拡大率

    if (m_enterFade)
    {
        const float t = (float)Time::ElapsedMs(m_fadeStart) / FADE_MS;
        float clamped = (t > 1.0f) ? 1.0f : t;

        m_circleScale = START_SCALE * (1.0f - clamped);

        if (clamped >= 1.0f)
        {
            m_circleScale = END_SCALE;
            m_enterFade = false;
        }
        return;
    }

    // 決定後フェード
    if (m_decided)
    {
        const float t = (float)Time::ElapsedMs(m_fadeStart) / FADE_MS;
        float clamped = (t > 1.0f) ? 1.0f : t;

        m_circleScale = START_SCALE * clamped;

        if (clamped >= 1.0f)
        {
            SceneManager::SetCurrentScene(
                (m_select == 0) ? "TutorialScene" : "TitleScene"
            );
        }
        return;
    }

    // パッド入力
    const auto pad = ReadPad();

    if (m_cursor)
        m_cursor->SetColor(pad.connected ? Color(1, 1, 1, 1) : Color(0.4f, 0.4f, 0.4f, 1));

    // メニュー操作
    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_W) || (pad.connected && pad.upTrg)) m_select--;
    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_S) || (pad.connected && pad.downTrg)) m_select++;

    m_select = std::clamp(m_select, 0, 1);

    const bool decide =
        CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN) ||
        (pad.connected && pad.aTrg);

    if (decide)
    {
        GetXAud()->soundSEPlay((int)SoundSEAssets::SeAccept);
        // ここでリセット
        EnemyCounter::GetInstance().ResetWave();
        EnemyCounter::GetInstance().ResetTotal();   // 累積も消す
        //UI::Get().ResetForNewRun();

        m_decided = true;
        m_circleScale = 0.0f;
        m_fadeStart = Time::Get().Now();
    }
}

void GameOverScene::draw(uint64_t)
{
    const float cx = Application::GetWidth() * 0.5f;
    const float cy = Application::GetHeight() * 0.5f;

    const Vector3 scale(1, 1, 1);
    const Vector3 rot(0, 0, 0);

    static constexpr float MENU_OFFSET_X = -300.0f;
    static constexpr float ACCEPT_OFFSET_X = 100.0f;
    static constexpr float ACCEPT_OFFSET_Y = -50.0f;

    const float menuX = cx + MENU_OFFSET_X;

    const float overX = cx - 20.0f;  // 中央より左
    const float overY = cy - 20.0f;

    const float row0Y = RowCenterY(0, cy);
    const float row1Y = RowCenterY(1, cy);
    const float selY = RowCenterY(m_select, cy);

    // ふよふよ
    const auto now = Time::Get().Now();
    const float tSec = (float)Time::ElapsedMs(m_bobStart, now) * 0.001f;

    static constexpr float BOB_A = 5.0f;
    static constexpr float BOB_HZ = 0.6f;
    const float bobY = std::sinf(tSec * 2.0f * PI * BOB_HZ) * BOB_A;

    // 背景＆Over
    if (m_bg)   m_bg->Draw(scale, rot, Vector3(cx, cy, 0));
    if (m_over) m_over->Draw(scale, rot, Vector3(overX, overY, 0));

    // プレート
    if (m_plate[0]) m_plate[0]->Draw(scale, rot, Vector3(menuX, row0Y, 0));
    if (m_plate[1]) m_plate[1]->Draw(scale, rot, Vector3(menuX, row1Y, 0));

    // 文字
    DrawMenuText("RETRY", menuX + TEXT_CENTER_OFFSET_X, row0Y);
    DrawMenuText("TITLE", menuX + TEXT_CENTER_OFFSET_X, row1Y);

    // カーソル
    if (m_cursor) m_cursor->Draw(scale, rot, Vector3(CursorX(menuX), selY + bobY, 0));

    // ACCEPT
    if (m_decided && m_accept)
        m_accept->Draw(scale, rot, Vector3(menuX + ACCEPT_OFFSET_X, selY + ACCEPT_OFFSET_Y + bobY, 0));

    // 円フェード
    if (m_circle && m_circleScale > 0.0f && (m_enterFade || m_decided))
    {
        const Vector3 s(m_circleScale, m_circleScale, 1.0f);
        m_circle->Draw(s, rot, Vector3(cx, cy, 0));
    }
}

void GameOverScene::dispose()
{
    XAudSound::GetInstance()->soundDispose();
}