#pragma once

#include	"../system/commontypes.h"
#include	"../system/utility.h"

// パーティクル
struct PARTICLE {
	Vector3	pos;			// 位置
	Vector3	firstvelocity;	// 初速度
	Vector3	velocity;		// 現在速度		
	Vector3	acceleration;	// 加速度		
	int life = 0;									// 寿命
	bool isAlive = false;							// 生存フラグ
};

// パーティクルエミッタ
class Emitter
{
	std::vector<PARTICLE> m_Particles;				// パーティクル群

	Vector3 m_Pos{};				// 位置
	Vector3 m_Direction;			// 方向

	Matrix4x4 m_DirectionMtx;		// パーティクルの射出方向（中心軸）

	// 円錐ベースで定義
	float m_Radius = 0.0f;							// パーティクルの発生範囲
	float m_Height = 0.0f;							// パーティクルの発生範囲
	unsigned int m_ParticelNum = 0;					// パーティクルの数

	// パーティクルのMAX寿命
	int m_MaxLife = 100;

	// 重力
	float m_Gravity = 0.1f;

public:
	// 開始
	void Start(
		Vector3 pos,
		unsigned int num,
		float radius,
		float height,
		float azimuth,
		float elevation,
		int maxlife,
		float gravity)
	{
		m_Pos = pos;
		m_ParticelNum = num;
		m_Radius = radius;
		m_Height = height;
		m_MaxLife = maxlife;
		m_Gravity = gravity;

		m_Particles.clear();
		m_Particles.resize(num);

		// 極座標系をオイラー角に変換（パーティクルの射出方向を生成する）
		CPolor3D polar(radius, elevation, azimuth);
		Vector3 targetpos = polar.ToCartesian();

		// パーティクルの射出方向（中心軸）を生成するクオータニオンを作成
		Quaternion quat = utility::CreateTargetQuaternion(
			Vector3(0, 1, 0),
			targetpos);

		// クオータニオンから行列を生成
		m_DirectionMtx = Matrix4x4::CreateFromQuaternion(quat);

		// 寿命決め
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distr(1, maxlife);

		// パーティクル初期化		
		std::vector<DirectX::SimpleMath::Vector3> points;
		points = utility::GeneratePointsInsideCone(
			radius,		// 円錐の半径
			height,		// 円錐の高さ
			num);		// 点の数

		m_Particles.clear();

		for (auto& p : points) {
			PARTICLE particle;

			particle.pos = m_Pos;

			particle.velocity = DirectX::SimpleMath::Vector3::Transform(p, m_DirectionMtx);
			particle.firstvelocity = particle.velocity;

			// 寿命を決める
			particle.life = distr(gen);

			m_Particles.push_back(particle);
		}
	}

	// 更新
	void Update() {

		// 寿命決め
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distr(1, m_MaxLife);

		for (auto& p : m_Particles) {

			p.life--;
			if (p.life <= 0) {
				p.velocity = p.firstvelocity;
				p.pos = m_Pos;
				p.life = distr(gen);
			}

			p.pos += p.velocity;
			p.velocity.y -= m_Gravity;
		}
	}

	const std::vector<PARTICLE>& GetParticles() {
		return m_Particles;				// パーティクル群
	}

