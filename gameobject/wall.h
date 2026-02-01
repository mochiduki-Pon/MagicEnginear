#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"
#include	"../system/CPlane.h"

class wall : public gameobject {

public:

	// 衝突した壁データ
	struct WallCollision {
		wall* wall;				// 壁オブジェクト	
		Vector3 Penetration;		// 侵入ベクトル
		Vector3 Sliding;			// 壁摺りベクトル
		Vector3 IntersectionPoint;	// 交点（最近接点）
	};

	wall() = default;
	wall(IScene* currentscene,
		SRT srt,
		float width,
		float height)
		: m_ownerscene(currentscene),
		gameobject(),
		m_width(width),
		m_height(height)
	{
	}

	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	void makeplanedata();

	PLANEINFO getplaneinfo();

	float getwidth() const;
	float getheight() const;

	bool isHit() {
		return (m_hitflag == true);
	}

	void clearhitflag() {
		m_hitflag = false;
	}

	void sethitflag(){
		m_hitflag = true;
	}
private:
	std::vector<Vector3>	m_vertices{};		// 壁頂点

	float					m_height = 100.0f;	// 幅
	float					m_width = 100.0f;	// 高さ

	CPlane					m_plane{};			// 平面の方程式

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
	// 対象と当たったかどうか
	bool m_hitflag = false;
};