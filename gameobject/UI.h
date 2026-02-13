#pragma once
#include <string>
#include <array>
#include <memory>
#include "BulletGimmick.h"
#include "../gameobject/player.h"

class DirectWrite;
class CSprite;

class UI {

    struct UIState
    {
        int freeBullet = 0;
        int aliveBullet = 0;
        int aliveTrap = 0;

        BulletGimmick::Element element = BulletGimmick::Element::Water;
        BulletGimmick::BulletMode mode = BulletGimmick::BulletMode::Shot;
        BulletGimmick::BulletSpec spec{};
    };

public:

    static UI& Get();

    void Init();
    void Update(uint64_t dt, int hp, int hpMax);
    void Draw();
    void DrawStageIndex(int stageIndex);
    void UpdateEnemyGauge(int remain, int total);

    void SetStageIndex(int stageIndex){ m_stageIndex = stageIndex; }
    void SetTrapMode(bool isTrap);// 弾モードください
	void SetMp(int mp) { m_mpUi = mp; }// MPください
    void SetCurrentBulletNo(BulletGimmick::BulletNo no);//ナンバー下さい

    //HPゲージが空かどうか
    bool IsHpVisualEmpty() const { return m_hpSmooth <= 0.5f; }

    //stage用数字スプライト
    std::array<std::unique_ptr<CSprite>, 10> m_digitSpr;

private:

    UI() = default;

    bool m_inited = false;      //初期化
    bool m_isTrapMode = false;  //Shot表示用

	// 敵ゲージ
    float m_enemyGaugeW = 0.0f;
    int   m_enemyTotalSeen = 1;  // Wave中の最大分母

    //スプライト
    std::unique_ptr<CSprite> m_hpplate;
    std::unique_ptr<CSprite> m_fillG;
	std::unique_ptr<CSprite> m_fillR;
    std::unique_ptr<CSprite> m_fillB;
    std::unique_ptr<CSprite> m_fillW;
    std::unique_ptr<CSprite> m_face;
    std::unique_ptr<CSprite> m_stage;
    std::unique_ptr<CSprite> m_256;
    std::unique_ptr<CSprite> m_shotT;
    std::unique_ptr<CSprite> m_shotTtrp;
    std::unique_ptr<CSprite> m_shotTsht;
    std::unique_ptr<CSprite> m_clearmin;
    std::unique_ptr<CSprite> m_resource;
    std::unique_ptr<CSprite> m_syabon;
    std::unique_ptr<CSprite> m_fireicon;
    std::unique_ptr<CSprite> m_hukidashi;

    int m_stageIndex = 1;       //stage
    float m_hp01 = 1.0f;        // 表示用
    float m_hpSmooth = 1.0f;    // 見栄え用
	int m_mpUi = 0;             // MP表示用
    float m_waveW = 260.0f;     // ゲージ現在幅

    BulletGimmick::BulletNo m_currentNo = BulletGimmick::BulletNo::WaterShot;
    UIState m_state{};

    void DrawMp(int mp);

    CMaterial					m_Material;					// マテリアル

};