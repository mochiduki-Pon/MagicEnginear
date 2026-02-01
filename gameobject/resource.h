#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CShader.h"
#include	"../system/CIndexBuffer.h"
#include	"../system/CVertexBuffer.h"
#include	"../system/CMaterial.h"
#include	"../system/CTexture.h"
#include	"../system/camera.h"
#include	"../main.h"
#include	"../system/Time.h"

class player;

class Resource : public gameobject {
public:

	void update(uint64_t delta)override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	void Spawn(const Vector3& pos, int life, bool enableAttract);
	void Attract(const Vector3& playerPos, uint64_t dtMs);
	void AttractToPlayer(uint64_t dtMs);
	void SetPlayer(player* p) { m_player = p; }

	Time::TimePoint m_spawnTime;

	struct EffectDesc {
		unsigned int num;
		float radius;
		float height;
		float azimuth;
		float elevation;
		int maxlife;
		float gravity;
	};

	struct Face {					// 三角形のインデックスデータ
		int		idx[3];
	};

	void drawbillboard(Camera*);

	//collision関連
	float GetCollisionRadius() const { return 20.0f; }
	Vector3 GetColliderCenter() const { return m_srt.pos + Vector3(0, GetCollisionRadius(), 0); }
	
	int GetValue() const { return m_value; }
	bool GetResourceLife() const { return m_alive; }
	void Kill() { m_alive = false; }
	void ResetLifeTime() { m_lifeTime = 0.0f; }

	//デバッグ用
	float GetLifeTime() const;
	float GetMaxLife() const;

private:

	player* m_player = nullptr;

	static constexpr float		FALL_Y = -190;
	static constexpr auto		RESOURCE_LIFE_TIME = std::chrono::seconds(60);
	static constexpr float ATTRACT_R = 200.0f; // 反応距離
	static constexpr float START_R = 100.0f;  // 実際に吸う距離
	static constexpr float SPEED = 150.0f;

	//リソース価値
	int m_value = 1;

	//billboardステータス
	bool m_alive = false;
	bool m_attract = false;
	float m_width = 10.0f;			// 幅
	float m_height = 10.0f;		// 高さ
	Vector3 m_dir;				// 進行方向
	float m_speed = 0.5f;		// 移動速度
	float m_lifeTime = 0.0f;	// 生成経過時間
	float m_maxLife = 10.0f;	// 消える時間

	// 描画
	CVertexBuffer<VERTEX_3D>	m_VertexBuffer;				// 頂点バッファ
	CShader* m_shader;										// シェーダー
	CMaterial					m_Material;					// マテリアル
	CTexture					m_Texture;					// テクスチャ

	std::vector<VERTEX_3D>		m_vertices;					// 頂点群
	std::vector<uint32_t>		m_indices;					// インデックス群

};