#pragma once

#include <array>
#include <memory>
#include <random>

#include "../system/camera.h"
#include "../system/IScene.h"
#include "../system/SceneClassFactory.h"
#include "../system/DirectWrite.h"
#include "../system/RandomEngine.h"
#include "../gameobject/field.h"
#include "../gameobject/wall.h"
#include "../gameobject/obstacle.h"
#include "../gameobject/player.h"
#include "../gameobject/enemy.h"
#include "../gameobject/tower.h"

#include "../utility/particle.h"
#include "../utility/blobshadow.h"
#include "../system/CSprite.h"
#include "../system/CStaticMesh.h"
#include "../system/CStaticMeshRenderer.h"
#include "../system/linedrawer.h"
#include "../system/WindowCounter.h"

#include "../utility/spline.h"
#include "../gameobject/PlayerBullet.h"
#include "../gameobject/UI.h"
#include "../gameobject/effectsystem.h"
#include "../gameobject/resource.h"

namespace spawnpoints
{
	//所持座標
	constexpr Vector3 SPAWN_ALL[] = {
		{ -150.0f, 0.0f, 800.0f },	//0
		{ -50.0f, 10.0f, 800.0f },	//1
		{   0.0f, 20.0f, 800.0f },	//2
		{  50.0f, 10.0f, 800.0f },	//3
		{  150.0f, 0.0f, 800.0f },	//4
		{ -60.0f, 0.0f, -800.0f },	//5
		{ -30.0f, 0.0f, -800.0f },	//6
		{   0.0f, 0.0f, -800.0f },	//7
		{  30.0f, 0.0f, -800.0f },	//8
		{  60.0f, 0.0f, -800.0f },	//9
	};

	static constexpr int COUNT = (int)std::size(SPAWN_ALL);
}

class TutorialScene : public IScene {
public:

	//WindowCounter m_killWindow;

	virtual ~TutorialScene() {}
	void SpawnPickup(const Vector3& pos, int count) override;

	//定数
	static constexpr uint32_t	WALLMAX = 10;
	static constexpr uint32_t	OBSTACLEMAX = 10;
	static constexpr uint32_t	ENEMYMAX = 10;
	static constexpr uint32_t	PLAYERBULLETMAX = 20;
	static constexpr float		FALL_Y = -190;
	static constexpr uint64_t GO_WAIT_MS = 400;
	static constexpr uint32_t	RESOURCEMAX = 30;
	//static constexpr uint32_t SMOKEMAX = 20;

	// コピコン不可
	TutorialScene(const TutorialScene&) = delete;

	/// 代入演算子不可
	TutorialScene& operator=(const TutorialScene&) = delete;

	explicit TutorialScene();
	void update(uint64_t deltatime) override;
	void draw(uint64_t deltatime) override;
	void init() override;
	void dispose() override;

	void debugUICamera();
	void resourceLoader();

	//getter
	tower* gettower() { return m_tower.get(); }			// タワー
	field* getfield()const { return m_field.get(); }	// フィールド
	player* getPlayer() { return m_player.get(); }		// プレイヤー

	//Array
	const std::array<std::unique_ptr<enemy>, ENEMYMAX>& GetEnemies() const{ return m_enemies; }

	//collision関連
	void CollisionStep();				// 衝突判定処理
	std::vector<wall*> getwalls() {
		std::vector<wall*> walls;
		for (const auto& wall : m_walls) {
			walls.push_back(wall.get());
		}
		return walls;
	}
	void GameOver();

	//shot関連
	PlayerBullet* TryReserveBullet();

	//
	std::unique_ptr<EffectSystem> m_effectExplosion;

private:

	//waveシナリオ
	enum class Stage { Stage1,	Stage2, Stage3, Stage4, Stage5, Count};
	enum class UiState : uint8_t { InGame, Clear };
	UiState m_uiState = UiState::InGame;
	Stage   m_stage = Stage::Stage1;
	Time::TimePoint m_stageStartTime;
	Time::TimePoint m_goFadeStart;

	struct WaveScript
	{
		uint64_t timeMs;		// この時刻になったら
		int      pointIndex;	// この番号に
		int      count;			// 数だけ出す
		enemy::EnemyState state;// type
	};

	//ステージスクリプト
	//Time/座標/数
	static constexpr WaveScript STAGE1_SCRIPT[] = {
	{ 400, 2, 1, enemy::EnemyState::Normal},};

	static constexpr WaveScript STAGE2_SCRIPT[] = {
	{ 400, 2, 1, enemy::EnemyState::Normal},
	{ 2000, 1, 1, enemy::EnemyState::Normal},
	{ 2000, 3, 1, enemy::EnemyState::Normal},
	{ 4000, 2, 1, enemy::EnemyState::Normal}, };

	static constexpr WaveScript STAGE3_SCRIPT[] = {
	{ 400, 1, 1, enemy::EnemyState::Normal},
	{ 400, 3, 1, enemy::EnemyState::Normal},
	{ 6000, 2, 1, enemy::EnemyState::Normal},
	{ 12000, 1, 1, enemy::EnemyState::Normal},
	{ 12000, 2, 1, enemy::EnemyState::Normal},
	{ 12000, 3, 1, enemy::EnemyState::Normal},};

	static constexpr WaveScript STAGE4_SCRIPT[] = {
	{ 400, 2, 1, enemy::EnemyState::Normal},
	{ 2000, 1, 1, enemy::EnemyState::Normal},
	{ 2000, 2, 1, enemy::EnemyState::Normal},
	{ 2000, 3, 1, enemy::EnemyState::Normal},
	{ 5000, 4, 1, enemy::EnemyState::Normal},
	{ 5000, 0, 1, enemy::EnemyState::Normal}, };

	static constexpr WaveScript STAGE5_SCRIPT[] = {
	{ 400, 2, 1, enemy::EnemyState::Ghost} };

	bool	m_cleared = false;			// フェード円表示
	bool	m_isGameOver = false;		// ゲームオーバー状態
	bool	m_clearAccepted = false;		// バッファ対策
	uint64_t m_clearTimerMs = 0;

	//ステージスクリプト関連
	const WaveScript* m_script = STAGE1_SCRIPT;
	size_t m_scriptCount = std::size(STAGE1_SCRIPT);
	size_t   m_scriptIndex = 0; // 次に処理する行
	uint64_t m_elapsedMs = 0;   // 経過時間
	int m_nextSpawnPoint = 0;
	float m_goCircleScale = 0.0f;
	int m_stageIndex = 1;

	void Clear();
	void SetupStage(Stage stage);
	void StartNextWave();
	int  GetStageIndex() const;		// 現在のステージ番号取得

	//soundflag
	bool m_audioInited = false;

	//sceneで使うもの
	std::unique_ptr<Camera> m_camera;		// カメラ
	std::unique_ptr<field> m_field;			// フィールド
	std::unique_ptr<player> m_player;		// プレイヤ
	std::unique_ptr<tower> m_tower;			// タワー
	std::unique_ptr<BlobShadow>	m_blobshadow;// 丸影
	std::unique_ptr<CSprite> m_circle;		// フェード円
	std::unique_ptr<DirectWrite> m_directwrite;// 文字

	//配列群z
	std::array<std::unique_ptr<enemy>, ENEMYMAX>	m_enemies;						// Enemy達
	std::array<std::unique_ptr<PlayerBullet>, PLAYERBULLETMAX>	m_playerBullets;	// PlayerBullet
	std::array<std::unique_ptr<wall>, WALLMAX>	m_walls;							// 壁群
	std::array<std::unique_ptr<obstacle>, OBSTACLEMAX>	m_obstacles;				// 障害物
	std::array<std::unique_ptr<Resource>, RESOURCEMAX>	m_resource;


	//maouse関連
	Vector3 m_pickuppos{ 0,0,0 };
	Vector3 m_farpoint{};
	Vector3 m_nearpoint{};

	FontData	m_fontdata;						// フォントデータ
	std::vector<SRT>	m_nodes{};				// ノード位置

	Vector3 m_camPos;
	bool    m_camInit = false;

	// 初回フラグ
	bool m_givenTutorialMp = false;

	// collision関連
	template <class PtrRange, class Pred>
	static bool AnyAlive(const PtrRange& r, Pred pred)
	{
		for (const auto& x : r)
			if (x && pred(*x)) return true;
		return false;
	}

	//当たり判定
	// タワー
	bool ResolveEnemyTowerCollision(enemy& e, const tower& t);
	static bool CollisionSphereCylinder_Push(
		const Call::Collision::BoundingSphere& sphere,
		const Call::Collision::BoundingCylinder& cylinder,
		Vector3& outPush);

	// リソース
	void SpawnResource(const Vector3& pos);		//resourceスポーン

	// Bullet関連
	uint32_t m_nextBullet = 0;
	//爆発
	bool m_hasExplosion = false;
	Vector3 m_exCenter{ 0,0,0 };
	float   m_exRadius = 0.0f;
	int     m_exDamage = 0;
	//爆発倍率関係
	float m_exDropMul = 1.0f;
	bool  m_exDropActive = false;
	float GetExplosionDropMul() const { return m_exDropActive ? m_exDropMul : 1.0f; }

	int GetFreeBulletCount() const;
	int GetAliveBulletCount() const;
	int GetAliveTrapCount() const;

};

REGISTER_CLASS(TutorialScene)

