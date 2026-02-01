#pragma once
#include <memory>
#include <array>
#include <algorithm>
#include "../system/CSprite.h"
#include "../system/IScene.h"
#include "../system/DirectWrite.h"
#include "../system/SceneClassFactory.h"
#include "../system/Time.h"

class GameOverScene : public IScene
{
public:
    GameOverScene(const GameOverScene&) = delete;
    GameOverScene& operator=(const GameOverScene&) = delete;
    explicit GameOverScene();

    void update(uint64_t delta) override;
    void draw(uint64_t delta) override;
    void init() override;
    void dispose() override;

private:
    FontData m_font{};

    // レイアウト定数
    static constexpr float PLATE_W = 512.0f * 0.5f;
    static constexpr float ROW_SPACING = 100.0f;
    static constexpr float BASE_Y_OFFSET = 180.0f;
    static constexpr float TEXT_PAD_L = 110.0f;
    static constexpr float TEXT_PAD_R = 30.0f;
    static constexpr float TEXT_HALF_H = 24.0f;
    static constexpr float CURSOR_X_INSET = 35.0f;
    static constexpr float TEXT_CENTER_OFFSET_X = -25.0f;
    static constexpr float START_SCALE = 8.0f;    // 円の最大拡大率
    static constexpr float END_SCALE = 0.0f;

    // 画像
    std::unique_ptr<CSprite> m_plate[2];
    std::unique_ptr<CSprite> m_cursor;
    std::unique_ptr<CSprite> m_accept;
    std::unique_ptr<CSprite> m_over;
    std::unique_ptr<CSprite> m_bg;
    std::unique_ptr<CSprite> m_circle;

    // 状態
    int  m_select = 0;          // 0: Retry, 1: Title
    bool m_decided = false;     // 決定したか
    bool m_enterFade = false;   //入り
    Time::TimePoint m_fadeStart{};
    float m_circleScale = 0.0f;

    float RowCenterY(int idx, float cy) const;
    D2D1_RECT_F PlateTextRect(float cx, float rowCY) const;
    float CursorX(float cx) const;
    void DrawMenuText(const char* text, float cx, float rowCY);
};

REGISTER_CLASS(GameOverScene);