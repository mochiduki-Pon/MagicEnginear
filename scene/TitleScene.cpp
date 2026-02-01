#include "../main.h"
#include "titlescene.h"
#include "../system/CDirectInput.h"
#include "../system/scenemanager.h"
#include "../scene/TutorialScene.h"
#include "../sound.h"


TitleScene::TitleScene()
{


}

void TitleScene::init()
{
    DirectWrite::GetInstance().Init(Renderer::GetSwapChain());

    m_font.fontSize = 32;
    m_font.Color = D2D1::ColorF(D2D1::ColorF::Silver);
    DirectWrite::GetInstance().SetFont(m_font);

    static const std::array<Vector2, 4> UV_FULL = {
        Vector2(0, 0), Vector2(1, 0),
        Vector2(0, 1), Vector2(1, 1)
    };

    m_img = std::make_unique<CSprite>(
        Application::GetWidth(), Application::GetHeight(),
        "assets/texture/Title2.png", UV_FULL
    );

    XAudSound::GetInstance()->soundInit();
    XAudSound::GetInstance()->soundBGMPlay
    ((int)SoundBGAssets::Title1);


    //m_plate[0] = std::make_unique<CSprite>(512 * 0.5f, 120 * 0.5f, "assets/texture/f1408_4.png", UV_FULL);
    //m_plate[1] = std::make_unique<CSprite>(512 * 0.5f, 120 * 0.5f, "assets/texture/f1409_2.png", UV_FULL);
    m_plateNormal[0] = std::make_unique<CSprite>(256, 60, "assets/texture/f1408_4.png", UV_FULL);
    m_plateSelected[0] = std::make_unique<CSprite>(256, 60, "assets/texture/f1408_41.png", UV_FULL);

    m_plateNormal[1] = std::make_unique<CSprite>(256, 60, "assets/texture/f1408_4.png", UV_FULL);
    m_plateSelected[1] = std::make_unique<CSprite>(256, 60, "assets/texture/f1408_41.png", UV_FULL);

    m_cursor = std::make_unique<CSprite>(50, 50, "assets/texture/914610.png", UV_FULL);
    m_accept = std::make_unique<CSprite>(390 * 0.4f, 228 * 0.4f, "assets/texture/Abotan.png", UV_FULL);

    m_circle = std::make_unique<CSprite>(256, 256, "assets/texture/syabon.png", UV_FULL);

    m_select = 0;
    m_decided = false;

    m_bobStart = Time::Get().Now();
    m_decided = false;
    m_fadeIn = true;
    m_fadeStart = Time::Get().Now();
    m_circleScale = 0.0f;
}

float TitleScene::RowCenterY(int idx, float cy) const
{
    const float y0 = cy + BASE_Y_OFFSET;
    return y0 + idx * ROW_SPACING;
}

D2D1_RECT_F TitleScene::PlateTextRect(float cx, float rowCY) const
{
    const float left = (cx - PLATE_W * 0.5f) + TEXT_PAD_L;
    const float right = (cx + PLATE_W * 0.5f) - TEXT_PAD_R;
    return D2D1_RECT_F{ left, rowCY - TEXT_HALF_H, right, rowCY + TEXT_HALF_H };
}

float TitleScene::CursorX(float cx) const
{
    return (cx - PLATE_W * 0.5f) + CURSOR_X_INSET;
}

void TitleScene::DrawMenuText(const char* text, float cx, float rowCY)
{
    DirectWrite::GetInstance().DrawString(
        std::string(text),
        PlateTextRect(cx, rowCY),
        D2D1_DRAW_TEXT_OPTIONS_NONE,
        false
    );
}
void TitleScene::update(uint64_t)
{
    static constexpr float IN_FADE_MS = 800.0f;
    static constexpr float IN_SCALE = 8.0f;
    static constexpr float DECIDE_WAIT_MS = 200.0f;

    const auto now = Time::Get().Now();

    if (m_fadeIn)
    {
        float t = (float)Time::ElapsedMs(m_fadeStart, now) / IN_FADE_MS;
        if (t > 1.0f) t = 1.0f;

        m_circleScale = IN_SCALE * t;

        if (t >= 1.0f)
        {
            m_fadeIn = false;
            m_circleScale = 0.0f;
        }
        return;
    }

    if (m_decided)
    {
        if (Time::ElapsedMs(m_decideStart, now) > DECIDE_WAIT_MS)
        {
            if (m_select == 0) SceneManager::SetCurrentScene("TutorialScene");
            else               SceneManager::SetCurrentScene("TitleScene");
        }
        return;
    }

    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_W)) {
        m_select--;
        GetXAud()->soundSEPlay((int)SoundSEAssets::Search);
    }
    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_S)) {
        m_select++;
        GetXAud()->soundSEPlay((int)SoundSEAssets::Search);
    }
    m_select = std::clamp(m_select, 0, 1);

    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN))
    {
        GetXAud()->soundSEPlay((int)SoundSEAssets::SeAccept);
        XAudSound::GetInstance()->soundBGMStop();

        m_decided = true;
        m_decideStart = now;
    }
}

void TitleScene::draw(uint64_t)
{
    const float cx = Application::GetWidth() * 0.5f;
    const float cy = Application::GetHeight() * 0.5f;

    const Vector3 scale(1, 1, 1);
    const Vector3 rot(0, 0, 0);

    static constexpr float MENU_OFFSET_X = 400.0f;
    static constexpr float ACCEPT_OFFSET_X = 100.0f;
    static constexpr float ACCEPT_OFFSET_Y = -50.0f;

    const float menuX = cx + MENU_OFFSET_X;

    const float row0Y = RowCenterY(0, cy);
    const float row1Y = RowCenterY(1, cy);
    const float selY  = RowCenterY(m_select, cy);

    // 背景
    if (m_img) m_img->Draw(scale, rot, Vector3(cx, cy, 0));

    // プレート
    CSprite* p0 = m_plateNormal[0].get();
    CSprite* p1 = m_plateNormal[1].get();
    if (m_decided)
    {
        if (m_select == 0) p0 = m_plateSelected[0].get();
        else               p1 = m_plateSelected[1].get();
    }
    if (p0) p0->Draw(scale, rot, Vector3(menuX, row0Y, 0));
    if (p1) p1->Draw(scale, rot, Vector3(menuX, row1Y, 0));

    // 文字
    DrawMenuText("START", menuX + TEXT_CENTER_OFFSET_X, row0Y);
    DrawMenuText("END",   menuX + TEXT_CENTER_OFFSET_X, row1Y);

    // ふわふわ
    const auto now = Time::Get().Now();
    const float tSec = (float)Time::ElapsedMs(m_bobStart, now) * 0.001f;

    static constexpr float BOB_A  = 5.0f;
    static constexpr float BOB_HZ = 0.6f;
    const float bobY = std::sinf(tSec * 2.0f * PI * BOB_HZ) * BOB_A;

    // カーソル
    if (m_cursor) m_cursor->Draw(scale, rot, Vector3(CursorX(menuX), selY + bobY, 0));

    // ACCEPT
    if (m_decided && m_accept)
        m_accept->Draw(scale, rot, Vector3(menuX + ACCEPT_OFFSET_X, selY + ACCEPT_OFFSET_Y + bobY, 0));

    // フェード円
    if (m_fadeIn && m_circle)
    {
        const Vector3 s(m_circleScale, m_circleScale, 1);
        m_circle->Draw(s, rot, Vector3(cx, cy, 0));
    }
}

void TitleScene::dispose()
{
    XAudSound::GetInstance()->soundDispose();
}
