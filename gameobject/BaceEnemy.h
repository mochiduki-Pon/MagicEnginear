//#pragma once
//#include	"gameobject.h"
//#include	"../system/IScene.h"
//#include	"../system/commontypes.h"
//#include	"../system/collision.h"
//#include	"../gameobject/wall.h"
//
//class BaceEnemy : public gameobject {
//
//public:
//	BaceEnemy(IScene* currentscene);
//	virtual ~BaceEnemy() = default;
//
//	void update(uint64_t delta) override;
//	void draw(uint64_t delta) override;
//	void init() override;
//	void dispose() override;
//
//protected:
// 
//	// オーナーSCENE
//	IScene* m_ownerscene = nullptr;
// std::unique_ptr<EnemyCounter> m_enemyCounter;
//
// 	//▼ステータス値▼
//	int m_maxHp = 0;
//	int m_hp = 0;
//	float		m_groundheight = 0.0f;
//	bool		m_onground = false;		// 接地フラグ
//	bool m_dead;
//	Vector3	m_move = { 0.0f,0.0f,0.0f };	// 移動量
//	Vector3	m_destrot = { 0.0f,0.0f,0.0f };	// 目標回転角度
//	Vector3	m_Velocity{ 0,0,0 };				// 速度
//	Vector3	m_Acceleration{ 0,0,0 };			// 加速度
//	Vector3	m_gravity{ 0,-2.8f,0 };				// 重力
//
//	// オーナーSCENE
//	IScene* m_ownerscene = nullptr;
//
//	//壁との当たり判定関連
//	bool wallshitcheck(std::vector<wall::WallCollision>& hitwalls);	// 壁とのヒットチェック
//	void ApplyWallAndMove();
//	void SeparateFromOthers();
//
//};