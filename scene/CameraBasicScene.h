#pragma once

#include <array>
#include <memory>

#include "../system/camera.h"
#include "../system/IScene.h"
#include "../system/SceneClassFactory.h"
#include "../system/DirectWrite.h"
#include "../system/RandomEngine.h"
#include "../gameobject/field.h"
#include "../gameobject/tower.h"
//#include "../gameobject/enemy.h"
#include "../gameobject/aim.h"
#include "../gameobject/player.h"
#include "../gameobject/obstacle.h"
#include "../gameobject/wall.h"

/**
 * @brief カメラ
 */
class CameraBasicScene : public IScene{
public:
	virtual ~CameraBasicScene() {}
	static constexpr uint32_t ENEMYMAX = 1;
	static constexpr uint32_t OBSTACLEMAX = 1;
	static constexpr uint32_t WALLMAX = 10;

	/// @brief コピーコンストラクタは使用不可
	CameraBasicScene(const CameraBasicScene&) = delete;

	/// @brief 代入演算子も使用不可
	CameraBasicScene& operator=(const CameraBasicScene&) = delete;

	/**
	 * @brief コンストラクタ
	 */
	explicit CameraBasicScene();

	/**
	 * @brief 毎フレームの更新処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 */
	void update(uint64_t deltatime) override;

	/**
	 * @brief 毎フレームの描画処理
	 * @param deltatime 前フレームからの経過時間（マイクロ秒）
	 *
	 */
	void draw(uint64_t deltatime) override;

	/**
	 * @brief シーンの初期化処理
	 *
	 */
	void init() override;

	/**
	 * @brief シーンの終了処理
	 *
	 */
	void dispose() override;

	/**
	 * @brief カメラの設定
	 *
	 */
	void debugUICamera();


	void debugFieldRemake();

	// フィールドに凸凹にする
	void debugFieldUnduration();

	// 平行光源
	void debugDirectionalLight();

	// タワーを取得
	tower* gettower() {
		return m_tower.get();
	}

	// リソースを読み込む
	void resourceLoader();

	// すべての壁を取得する
	std::vector<wall*> getwalls() {
		std::vector<wall*> walls;
		for (const auto& wall : m_walls) {
			walls.push_back(wall.get());
		}
		return walls;
	}

	// すべての障害物を取得する
	std::vector<obstacle*> getobstacles() {
		std::vector<obstacle*> obstacles;
		for (const auto& obs : m_obstacles) {
			obstacles.push_back(obs.get());
		}
		return obstacles;
	}

	// フィールドを取得する
	field* getfield()const {
		return m_field.get();
	}

private:
	/**
	* @brief このシーンで使用するカメラ
	*/
   std::unique_ptr<Camera> m_camera;

   //Player
   std::unique_ptr<player> m_player;

   //タワー
   std::unique_ptr<tower> m_tower;

   //フィールド
   std::unique_ptr<field> m_field;

   //UI
   std::unique_ptr<aim> m_aim;

   //enemy
   //std::array<std::unique_ptr<enemy>, ENEMYMAX>	m_enemies;	// 敵

   /**
   * @brief 障害物群
   */
   std::array<std::unique_ptr<obstacle>, OBSTACLEMAX>	m_obstacles;	// 障害物

   /**
   * @brief 壁群
   */
   std::array<std::unique_ptr<wall>, WALLMAX>	m_walls;		// 壁群

   /**
   * @brief マウスでピックアップした位置
   */
   Vector3 m_pickuppos{0,0,0};
   Vector3 m_farpoint{};
   Vector3 m_nearpoint{};

   // DirectWrite
   std::unique_ptr<DirectWrite> m_directwrite;

   // フォントデータ
   FontData	m_fontdata;

};

REGISTER_CLASS(CameraBasicScene)