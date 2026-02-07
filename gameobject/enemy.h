#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CAnimationObject.h"
#include	"../system/CAnimationMesh.h"
#include	"../system/CAnimationdata.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"
#include	"../gameobject/field.h"

class enemy : public gameobject {

public:
	enemy(IScene* currentscene)

		: m_maxHp(3),
		m_hp(m_maxHp),

		m_shader(nullptr),
		m_mesh(nullptr),
		m_ownerscene(currentscene) {
	}

	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

protected:

	//▼ステータス値▼
	int m_maxHp = 0;
	int m_hp = 0;
	bool m_dead = false;
	// オーナーSCENE
	IScene* m_ownerscene = nullptr;

public:

	Time::TimePoint m_nextTowerHit = Time::Clock::time_point{};

	enum class Status : uint8_t { Alive, Dead };
	enum class Grande : uint8_t { Graund, Air };
	//enum class EnemyType : uint8_t { Normal, Fast, Tank };
	enum class Reaction : uint8_t { None, Stun, Knockback, Damage };

	// ▼状態▼
	Status GetStatus() const { return m_sts; }
	Reaction GetReAct()const { return m_reaction; }
	bool IsAlive() const { return m_sts == Status::Alive; }

	// ▼外部呼び出し▼
	void Kill();				//kill
	void Damage(int amount);
	void SetHitFromExplosion(bool v) { m_hitFromExplosion = v; }
	//void ForceInactiveState();//遺体掃除

	// スポーン管理用
	void SetSpawned(bool v);
	bool m_spawned = false;

	// 弾当たり判定用（即席ポテト）
	float GetCollisionRadius() const { return m_collisionRadius; }
	void SetCollisionRadius(float r) { m_collisionRadius = r; }
	int GetAttackPower() const { return m_attackPower; }//タワー時
	Vector3 GetColliderCenter() const;				//当たり判定の位置
	void Spawn(const Vector3& pos);
	//void SetHitKnock(float h, float v) { m_hitKbH = h; m_hitKbV = v; }
	bool m_hitFromExplosion = false;

	bool wallshitcheck(std::vector<wall::WallCollision>& hitwalls);	// 壁とのヒットチェック
	void ApplyWallAndMove();

	// 爆発ダメージ倍率設定
	void SetExplosionDropMul(float mul) { m_explosionDropMul = mul; }
	float GetExplosionDropMul() const { return m_explosionDropMul; }

	// タワー当り管理
	bool CanHitTower(Time::TimePoint now) const { return now >= m_nextTowerHit; }
	void SetNextTowerHit(Time::TimePoint t) { m_nextTowerHit = t; }


private:

	enum class DeathCause : uint8_t { Normal, Explosion, Fall };

	Status m_sts = Status::Alive;;
	Reaction m_reaction = Reaction::None;
	DeathCause m_deathCause = DeathCause::Normal;
	BulletGimmick::BulletNo m_lastHitBulletNo = BulletGimmick::BulletNo::WaterShot;

	// ▼定数パラメータ▼
	const float VALUE_MOVE_MODEL = 10.0f;					// 移動速度
	const float VALUE_ROTATE_MODEL = PI * 0.02f;			// 回転速度
	const float RATE_ROTATE_MODEL = 0.40f;					// 回転慣性係数
	const float RATE_MOVE_MODEL = 0.20f;					// 移動慣性係数
	static constexpr float HIT_FLASH_MAX_MULTIPLIER = 2.5f;	// ダメージ点滅最大倍率
	static constexpr uint64_t DAMAGE_FLASH_MS = 120;		// ダメージ点滅時間(ms)
	static constexpr uint64_t DEAD_VISIBLE_MS = 600;		// 死体残留時間(ms)
	static constexpr float ENEMY_FALL_Y = -190.0f;
	static constexpr uint64_t DAMAGE_REMAIN_MS = 100;		// ダメージリアクション時間(ms)

	//ID管理
	inline static uint32_t s_nextId = 1;
	uint32_t m_id = 0;

	float m_explosionDropMul = 1.0f;
	int BaseDropRand();

	Vector3 m_targetPos{ 0,0,0 };

	// 状態ごとメソッド
	void Alive();
	void OnDamage();
	void EnterDeath();
	void Air();
	void Ground();
	void HitBullet(const BulletGimmick::BulletSpec&, const Vector3&);
	void SeparateFromOthers();

	void SetLastHitBulletNo(BulletGimmick::BulletNo no)
	{	m_lastHitBulletNo = no; }

	//戦闘関連
	int m_attackPower = 5;   // 敵ごとの攻撃力
	Cooldown m_attackCD;     // 攻撃間隔
	bool m_deathEntered = false;
	bool m_damageImpulseDone = false;
	bool m_pendingKill = false;			//Kill確定
	bool m_landedOnce = false;
	float m_kbH = 0.0f;		//ノックバックデフォ値水平
	float m_kbV = 0.0f;		//ノックバックデフォ値垂直
	int m_boostFrames = 0;

	//タワー当り
	uint64_t m_nextTowerHitMs = 0;

	// 遠距離当たり判定半径（即席ポテト）
	float m_collisionRadius = 20.0f;

	float m_speed = 0.5f;					// 移動速度	
	Vector3	m_move = { 0.0f,0.0f,0.0f };	// 移動量
	Vector3	m_destrot = { 0.0f,0.0f,0.0f };	// 目標回転角度
	Vector3	m_Velocity{ 0,0,0 };				// 速度
	Vector3	m_Acceleration{ 0,0,0 };			// 加速度
	Vector3	m_gravity{ 0,-2.8f,0 };				// 重力
	float		m_groundheight = 0.0f;
	bool		m_onground = false;		// 接地フラグ

	//▼描画▼
	//タイマー
	uint64_t m_damageRemainMs = 0;			// ダメージ点滅残り時間
	uint64_t m_deadRemainMs = 0;			// 死体残留時間
	CStaticMesh* m_mesh{};
	CStaticMeshRenderer* m_meshrenderer{};
	CShader* m_shader{};
	CStaticMeshRenderer* m_staticmeshrenderer{};
	CShader* m_staticmeshshader{};

};