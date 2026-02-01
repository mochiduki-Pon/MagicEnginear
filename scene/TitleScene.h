#pragma once

#include "../system/IScene.h"
#include "../system/CSprite.h"
#include "../system/camera.h"
#include "../system/SceneClassFactory.h"
#include "../system/DirectWrite.h"
#include "../scene/SceneFade.h"
#include "../system/Time.h"

class TitleScene : public IScene {
public:
    TitleScene(const TitleScene&) = delete;
    TitleScene& operator=(const TitleScene&) = delete;

    explicit TitleScene();
    void update(uint64_t deltatime) override;
    void draw(uint64_t deltatime) override;
    void init() override;
    void dispose() override;

private:
    FontData m_font{};
    Time::TimePoint m_fadeStart{};
    Time::TimePoint m_decideStart{};
    Time::TimePoint m_bobStart;

    std::unique_ptr<CSprite> m_img;
    std::unique_ptr<CSprite> m_plateNormal[2];
    std::unique_ptr<CSprite> m_plateSelected[2];
    //std::array<std::unique_ptr<CSprite>, 2> m_plate;
    std::unique_ptr<CSprite> m_cursor;
    std::unique_ptr<CSprite> m_accept;
    std::unique_ptr<CSprite> m_circle;

    static constexpr float PLATE_W = 512.0f * 0.5f;
    static constexpr float ROW_SPACING = 100.0f;
    static constexpr float BASE_Y_OFFSET = 180.0f;
    static constexpr float TEXT_PAD_L = 110.0f;
    static constexpr float TEXT_PAD_R = 30.0f;
    static constexpr float TEXT_HALF_H = 24.0f;
    static constexpr float CURSOR_X_INSET = 35.0f;
    static constexpr float TEXT_CENTER_OFFSET_X = -25.0f;

    int  m_select = 0;          // 0: End, 1: Title
    bool m_decided = false;     // 決定したか
    float m_circleScale = 0.0f;
    bool  m_fadeIn = false;
	float m_cursorPhase = 0.0f; // カーソルの点滅用

    float RowCenterY(int idx, float cy) const;
    D2D1_RECT_F PlateTextRect(float cx, float rowCY) const;
    float CursorX(float cx) const;
    void DrawMenuText(const char* text, float cx, float rowCY);
};

REGISTER_CLASS(TitleScene);