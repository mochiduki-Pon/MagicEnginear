#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CShader.h"
#include    "../system/CIndexBuffer.h"
#include    "../system/CVertexBuffer.h"
#include	"../system/CMaterial.h"
#include	"../system/CTexture.h"
#include	"../system/CPlane.h"
#include    "../system/collision.h"
#include    "../system/camera.h"
#include "../system/IScene.h"
#include	"../gameobject/gameobject.h"
#include "BulletGimmick.h"
#include "../system/Time.h"
class enemy;

class PlayerBullet : public gameobject {
public:
	void update(uint64_t delta)override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	struct Face {
		int		idx[3];
	};

	struct VisualState
	{
		Color top = Color(1, 1, 1, 1);
		Color bottom = Color(0, 0, 1, 1);
		float baseScale = 1.0f;
		bool pulse = false;
		float pulseSpeed = 8.0f;
		float pulseAmp = 0.2f;
		float age = 0.0f;
	};

	VisualState m_vis;

	void drawbillboard(Camera*);

	bool GetisAlive() const { return isAlive; }
	void SetisAlive(bool isAl) { isAlive = isAl; }
	void Kill() { SetisAlive(false); }

	void SetDir(const Vector3& dir) { m_dir = dir; } // 正規化
	void SetSpeed(float s);

	const Vector3& GetDir() const { return m_dir; }
	void ResetLifeTime() { m_lifeTime = 0.0f; }

	BulletGimmick::BulletNo m_selectedNo;
	BulletGimmick::BulletNo GetNo() const { return m_no; }

	// 発射設置初期化
	void Spawn(const Vector3& pos, const Vector3& dir, BulletGimmick::BulletNo no);
	void SetActorNo(uint8_t no) { m_actorNo = no; }
	uint8_t GetActorNo() const { return m_actorNo; }
	bool IsTrap() const{ return m_spec.isTrap; }

	// 弾当たり判定用
	float GetCollisionRadius() const { return m_collisionRadius; }
	void SetCollisionRadius(float r) { m_collisionRadius = r; }
	Vector3 GetColliderCenter() const;
	float GetExplosionRadius() const;

	//デバッグ用
	float GetLifeTime() const;
	float GetMaxLife() const;


	int GetDamage() const { return BulletGimmick::Spec(m_no).damage; }
	Vector3 GetExplosionCenter() const;
	void Explode();

private:

	BulletGimmick::BulletNo   m_no = BulletGimmick::BulletNo::WaterShot;
	BulletGimmick::BulletSpec m_spec{};
	Time::TimePoint m_spawnAt{};   // 生存開始時刻

	uint64_t m_maxLifeMs = 0;      // 寿命(ms)

	bool isAlive = false;
	bool     m_triggered;  // Trapが1回発動済みか

	float m_width = 10.0f;
	float m_height = 10.0f;
	Vector3 m_dir;
	float m_speed = 0.5f;
	float m_lifeTime = 0.0f;
	float m_maxLife = 40.0f;
	float m_tick = 0.0f;
	float m_visAge = 0.0f;

	// 当たり判定効果用
	float m_radius = 0.0f;

	// 遠距離当たり判定半径
	float m_collisionRadius = 10.0f;

	uint8_t m_actorNo;//弾のアクターナンバー

	void ResetForSpawn();
	void VisualGimmick();

	CVertexBuffer<VERTEX_3D>	m_VertexBuffer;

	CShader*					m_shader;
	CMaterial					m_Material;
	CTexture					m_Texture;

	std::vector<VERTEX_3D>		m_vertices;
	std::vector<uint32_t>		m_indices;

	inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
	{
		return a * (1.0f - t) + b * t;
	}
};