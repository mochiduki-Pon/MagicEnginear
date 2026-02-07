#pragma once

#include	<memory>
#include <array>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"
#include    "wall.h"
#include    "../system/Cooldown.h"
#include    "../utility/MathUtil.h"
#include    "BulletGimmick.h"

class player : public gameobject {

public:

	player(IScene* currentscene)
		:
		m_maxHp(3),
		m_hp(m_maxHp),
		m_maxMp(300),
		m_mp(0),
		m_collisionRadius(15.0f),
		m_selectedNo(BulletGimmick::BulletNo::WaterShot),

		m_mesh(nullptr),
		m_meshrenderer(nullptr),
		m_shader(nullptr),
		m_ownerscene(currentscene) {
	}

	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	enum class PlayerState : uint8_t { Alive, Dead };
	enum class PlayerActState : uint8_t { Idle, Walk, Jump, Attack };
	enum class PlayerReact : uint8_t { None, Damage, };
	enum class BulletMode : uint8_t { Shot, Trap };

	PlayerReact GetReAct()const { return m_react; }
	BulletGimmick::BulletNo GetSelectedNo() const;

	// 動きのパラメータ
	const float VALUE_MOVE_MODEL = 0.7f;					// 移動速度
	const float VALUE_ROTATE_MODEL = PI * 0.02f;			// 回転速度
	const float RATE_ROTATE_MODEL = 0.40f;					// 回転慣性係数
	const float RATE_MOVE_MODEL = 0.20f;					// 移動慣性係数

	Vector3 getmove() const { return m_move; }

	float getwidth() const { return m_width; }
	float getheight() const { return m_height; }
	float getdepth() const { return m_depth; }
	float GetCollisionRadius() const { return m_collisionRadius; }

	Vector3 GetPlayerDir() { return m_vDir; }
	Vector3& GetPlayerMove() { return m_move; }

	void SetPlayerDir(Vector3 vDir) { m_vDir = vDir; }
	bool ConsumeShotFlag();
	void ChangeBullet();

	bool m_shotFlg = false;							// ショットフラグ
	bool obstacleshitcheck();								// 障害物とのヒットチェック
	bool wallshitcheck(std::vector<wall::WallCollision>& hitwalls);	// 壁とのヒットチェック

	Vector3 GetVelocity() const { return m_Velocity; }
	Vector3 GetAcceleration() const { return m_Acceleration; }

	Vector3 GetColliderCenter() const;				//当たり判定の位置
	void Knockback(const Vector3& fromPos);

	//▼Playerstatus▼
	int GetHP() const { return m_hp; }
	int GetHpMax() const { return m_maxHp; }
	bool IsDead() const { return m_hp <= 0; }
	int GetMp() const { return m_mp; }
	int GetMaxMp() const { return m_maxMp; }
	void AddMp(int amount);

	// 無敵時間つきダメージ
	bool Damage(int amount);

	//プロトタイプ
	void PlayerHandle();
	void JumpHandle();
	void ShotHandle();
	void OnDamage();
	void CheckShotDir();

	//Status
	void ResetStatus() { m_hp = m_maxHp;	m_dmgCD.Reset(); }
	bool ConsumeMp(int cost);
	bool CanUseTrap(int cost) const;

	PlayerState GetState() const { return m_playersts; }

	//debug
	PlayerActState m_prevActState = PlayerActState::Idle;

private:


	PlayerState  m_playersts = PlayerState::Alive;
	PlayerReact m_react = PlayerReact::None;
	PlayerActState m_actstate = PlayerActState::Idle;
	BulletGimmick::BulletNo m_no = BulletGimmick::BulletNo::WaterShot;
	BulletGimmick::BulletNo m_selectedNo;
	BulletGimmick::Element m_element = BulletGimmick::Element::Water;
	BulletMode   m_bulletMode = BulletMode::Shot;

	static constexpr size_t ToIndex(BulletMode mode)
	{
		return static_cast<size_t>(mode);
	}

	static constexpr uint64_t RecastMs(BulletGimmick::BulletNo no)
	{
		return BulletGimmick::Spec(no).isTrap ? 1000 : 700; // ← まずは現状維持
	}

	void HandleElement();

	// チューニング
	static constexpr uint8_t	BulletModeCount = 2;		//shot,trap
	static constexpr float HIT_FLASH_MAX_MULTIPLIER = 2.5f;	// ダメージ点滅最大倍率
	static constexpr uint64_t DAMAGE_FLASH_MS = 120;		// ダメージ点滅時間(ms)
	static constexpr uint64_t DEAD_VISIBLE_MS = 600;		// 死体残留時間(ms)
	//static constexpr int		HoleDamage = 1;				//将来穴→リスポ用
	//static constexpr float		HoleRespawnWait = 2.5f;		//穴→リスポ待ち時間(sec)

	std::array<Cooldown, BulletModeCount> m_bulletcd{};

	//▼Playerstatus▼
	int		m_maxHp;
	int		m_hp;
	int		m_maxMp;
	int		m_mp;
	int		m_mpDelta = 0;
	float m_collisionRadius;

	int ConsumeMpDelta();

	// 被弾無敵（Cooldownで統一）
	Cooldown m_dmgCD{};
	static constexpr uint64_t DmgInvincible = 800;

	float m_width{};
	float m_height{};
	float m_depth{};

	Vector3 m_spawnPos{ 0,100,0 };
	Vector3	m_move = { 0.0f,0.0f,0.0f };		// 移動速度
	Vector3	m_destrot = { 0.0f,0.0f,0.0f };		// 回転角度
	Vector3 m_vDir = { 0,0,0 };					// 方向
	Vector3	m_Velocity{ 0,0,0 };				// 速度
	Vector3	m_Acceleration{ 0,0,0 };			// 加速度
	Vector3	m_gravity{ 0,-2.8f,0 };				// 重力

	Vector3 shotDir = { 0,0,0 };				//shot方向
	Vector3 shotPos = { 0,0,0 };				//shot位置

	float		m_radius = 10.0f;
	float		m_e = 1.0f;				// 反発係数
	float		m_groundheight = 0.0f;	// 摩擦係数
	bool		m_onground = false;		// 接地フラグ
	bool		m_jumpFlg = false;		// ジャンプフラグ

	bool		m_holeSequenceActive = false;	// 穴演出フラグ
	float		m_holeSequenceTimer = 0.0f;   // sec
	Vector3		m_holeEnterPos{};           // 演出開始位置固定

	//描画
		//タイマー
	uint64_t m_damageRemainMs = 0;			// ダメージ点滅残り時間
	uint64_t m_deadRemainMs = 0;			// 死体残留時間

	CStaticMesh* m_mesh;
	CStaticMeshRenderer* m_meshrenderer;
	CShader* m_shader;

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;

};