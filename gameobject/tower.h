#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"
#include	"../system/collision.h"
#include	"../system/Time.h"
#include	<iostream>
//#include    "../system/CDirectInput.h"

class tower : public gameobject {

public:
	tower(IScene* currentscene)
		: m_staticmeshshader(nullptr),
		m_staticmeshrenderer(nullptr),
		m_staticmesh(nullptr),
		//m_mesh(nullptr),
		//m_meshrenderer(nullptr),
		m_ownerscene(currentscene) {
	}

	virtual ~tower() {}
	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	//外部伝達
	void Reboot();					// タワーダメージ開始
	bool UnderAtack(int amount); // ダメージを受けられたら true
	void OnDamage();                         // 被弾演出開始
	bool IsDead() const { return m_hp <= 0; }
	bool IsEnd() const;

	void OnDMF();
	void ApplyDrawColor() const;

	//collision用のシリンダー取得
	Call::Collision::BoundingCylinder GetCylinder() const;
	void SetCollider(float radius, float height) { m_colRadius = radius; m_colHeight = height; }

private:

	enum class Status : uint8_t { Activate, Dead };
	enum class VisualState : uint8_t { Normal, Danger, Break, DMF };
	Status m_sts = Status::Activate;
	VisualState m_vstate = VisualState::Normal;

	static constexpr uint64_t DAMAGE_FLASH_MS = 120;			//hitEffect持続時間(ms)
	static constexpr float HIT_FLASH_MAX_MULTIPLIER = 1.8f;

	// 当たり判定パラメータ
	float m_colRadius = 18.0f;
	float m_colHeight = 120.0f;
	bool m_cantAct = false;				// Attack受付不能時間
	bool m_contactPrev = false;			// 前フレーム接触してたか（立ち上がり検知）
	bool m_callalarm = false;			// Scene→音鳴らせ

	int  m_hp = 100;
	int  m_maxHp = 100;

	float m_holeHeight = -200.0f; // なにこれ何でここにあるのか不明

	uint64_t m_vstateRemainMs = 0;
	uint64_t m_damageCDMs = 500;		// 例：0.5秒ごとに減る
	uint64_t m_damageRemainMs = 0;		// 残り（Time utilがあるならそれで）
	uint64_t m_nextDamageTime = 0;		//dmcool
	int  m_damagePerTick = 1;
	float m_OffsetY = 40.0f;

	bool IsAlive() const { return m_sts == Status::Activate; }

	CStaticMesh* m_staticmesh{};
	CStaticMeshRenderer* m_staticmeshrenderer{};
	CShader* m_staticmeshshader{};
	CMaterial				m_Material;

	IScene* m_ownerscene = nullptr;
};