//#pragma once
//#include "../system/PlaneDrawer.h"
//#include "../main.h"
//
//class ScreenFade {
//public:
//    enum class Mode {
//        FadeIn,   // 黒 → 透明
//        FadeOut   // 透明 → 黒
//    };
//
//    // フェード開始
//    void Start(Mode mode, float speed = 1.0f) {
//        m_active = true;
//        m_mode = mode;
//        m_speed = speed;
//
//        if (mode == Mode::FadeIn) {
//            m_alpha = 1.0f;   // 最初は真っ黒
//        }
//        else {
//            m_alpha = 0.0f;   // 最初は透明
//        }
//    }
//
//    // dt = 秒
//    void Update(float dt) {
//        if (!m_active) return;
//
//        if (m_mode == Mode::FadeIn) {
//            m_alpha -= m_speed * dt;
//            if (m_alpha <= 0.0f) {
//                m_alpha = 0.0f;
//                m_active = false;
//            }
//        }
//        else { // FadeOut
//            m_alpha += m_speed * dt;
//            if (m_alpha >= 1.0f) {
//                m_alpha = 1.0f;
//                m_active = false;
//            }
//        }
//    }
//
//    void Draw() const {
//        if (!m_active) return;
//
//        PlaneDrawerDraw(
//            Vector3(0, 0, 0),               // 回転なし
//            SCREEN_WIDTH, SCREEN_HEIGHT,    // 全画面
//            Color(0, 0, 0, m_alpha),        // 黒フェード（αだけ変える）
//            SCREEN_WIDTH / 2.0f,
//            SCREEN_HEIGHT / 2.0f,
//            0.0f
//        );
//    }
//
//    bool IsActive() const { return m_active; }
//
//private:
//    bool  m_active = false;
//    float m_alpha = 1.0f;
//    float m_speed = 1.0f;
//    Mode  m_mode = Mode::FadeIn;
//};
