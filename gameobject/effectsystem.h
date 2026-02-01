#pragma once

#include	<memory>
#include	<array>
#include	"gameobject.h"
#include	"../system/CShader.h"
#include	"../system/CIndexBuffer.h"
#include	"../system/CVertexBuffer.h"
#include	"../system/CMaterial.h"
#include	"../system/CTexture.h"
#include	"../system/SphereDrawer.h"
#include	"../system/camera.h"
#include	"../main.h"
#include	"../system/Time.h"

enum class TitleState { Idle, FadingOut };

class EffectSystem : public gameobject {
public:

	void update(uint64_t delta)override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	Time::TimePoint m_exStart{};

	enum class EffectNo : uint8_t { Smoke,Bubble };

	//struct Face {					// 三角形のインデックスデータ
	//	int		idx[3];
	//};

	void drawbillboard(Camera*);

	void StartExplosion(const Vector3& pz);
	//bool GeteffectLife() { return isAlive; }
	//void SeteffectLife(bool isAl) { isAlive = isAl; }
	//void Vanish() { SeteffectLife(false); }
	//void ResetLifeTime() { m_lifeTime = 0.0f; }
	void SetSpeed(float s);

	//デバッグ用
	float GetLifeTime() const;
	float GetMaxLife() const;

private:

	struct Explosion
	{
		bool alive = false;
		Vector3 pos{};
		Time::TimePoint start{};
	};

	static constexpr int MAX_EX = 64;

	//爆発スフィア
	bool    exploding = false;
	Vector3 pos = Vector3(0, 0, 0);
	float   lifeMs = 0.0f;
	float   maxSpLifeMs = 300.0f;
	float   maxSpRadius = 200.0f;
	Color   color = Color(1, 0.2f, 0.0f, 1.0f);

	//爆発エフェクト用
	bool m_lightAlive = false;
	Time::TimePoint m_lightStart{};
	float m_lightLifeMs = 250.0f;   // 再生時間
	float m_lightMaxScaleX = 10.0f; // 伸び倍率
	Vector3 m_lightPos{};

	////billboardステータス
	//bool isAlive = false;
	//float m_width = 10.0f;			// 幅
	//float m_height = 10.0f;		// 高さ
	//Vector3 m_dir;				// 進行方向
	//float m_speed = 0.5f;		// 移動速度
	//float m_lifeTime = 0.0f;	// 生成経過時間
	//float m_maxLife = 10.0f;	// 消える時間

	//エフェクト群
	std::array<Explosion, MAX_EX> m_ex;

	// 描画
	CVertexBuffer<VERTEX_3D>	m_VertexBuffer;				// 頂点バッファ
	CShader* m_shader;										// シェーダー
	CMaterial					m_Material;					// マテリアル
	CTexture					m_Texture;					// テクスチャ

	std::vector<VERTEX_3D>		m_vertices;					// 頂点群
	std::vector<uint32_t>		m_indices;					// インデックス群

};