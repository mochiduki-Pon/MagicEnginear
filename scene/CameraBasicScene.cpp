#include "CameraBasicScene.h"
#include "../system/CDirectInput.h"
#include "../utility/ScreenToWorld.h"
#include "../system/SphereDrawer.h"
#include "../system/DebugUI.h"
#include "../system/CPolar3D.h"
#include "../gameobject/tower.h"
#include "../system/meshmanager.h"

namespace {
}

/**
 * @brief コンストラクタ
 */
CameraBasicScene::CameraBasicScene()
{
}

// 平行光源の方向セット
void CameraBasicScene::debugDirectionalLight()
{
	static Vector4 direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f); // Z軸+方向に光を当てる	

	ImGui::Begin("debug Directional Light");

	ImGui::SliderFloat3("direction ", &direction.x, -1, 1);
	direction.Normalize();										// 正規化

	LIGHT light{};
	light.Enable = true;
	light.Direction = direction;

	light.Direction.Normalize();
	light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

	Vector4 Direction = Vector4(direction.x, direction.y, direction.z, 0.0f);
	Renderer::SetLight(light);

	ImGui::End();
}

/**
 * @brief カメラの設定
 */
void CameraBasicScene::debugUICamera() {

	static float azimuth = PI/2.0f;
	static float elevation = 0.0f;
	static float radius = 1000.0f;

	ImGui::Begin("debug Camera");

	Vector3 camerapos = m_camera->GetPosition();
	Vector3 lookatpos = m_camera->GetLookat();
	Vector3 up = m_camera->GetUP();

	ImGui::SliderFloat3("camera pos ", &camerapos.x, -PI, PI);
	ImGui::SliderFloat3("camera up  ", &up.x, -PI, PI);

	ImGui::Separator();
	ImGui::Separator();

	ImGui::SliderFloat("azimuth ", &azimuth,-PI,PI);
	ImGui::SliderFloat("elevation ", &elevation, -PI/2.0f, PI / 2.0f);
	ImGui::SliderFloat("radius ", &radius, 100.0f, 3000.0f);

	CPolor3D polor(radius, elevation,azimuth);
	Vector3 cpos = polor.ToCartesian();

	CPolor3D polorup(1.0f, elevation+PI/2.0f,azimuth);
	Vector3 upvector = polorup.ToCartesian();;

	m_camera = std::make_unique<Camera>(
		cpos+lookatpos, 
		lookatpos,
		upvector);

	ImGui::Separator();
	ImGui::Separator();
	ImGui::SliderFloat3("nearpoint ", &m_nearpoint.x, -10000.0f, 10000.0f);
	ImGui::SliderFloat3("farpoint ", &m_farpoint.x, -10000.0f, 10000.0f);

	ImGui::Separator();
	ImGui::Separator();
	ImGui::SliderFloat3("pickuppos ", &m_pickuppos.x, -10000.0f, 10000.0f);
	
	ImGui::End();
}

// フィールド再作成
void CameraBasicScene::debugFieldRemake() {

	ImGui::Begin("debug Field Remake");

	static int dividex = 10;
	static int dividez = 10;

	static float width = 500.0f;
	static float depth = 500.0f;

	ImGui::SliderFloat("width", &width, 10, 1000);
	ImGui::SliderFloat("depth", &depth, 10, 1000);
	ImGui::SliderInt("devide x", &dividex, 1, 50);
	ImGui::SliderInt("devide z", &dividez, 1, 50);

	if (ImGui::Button("recreate  field")) {

		// フィールド初期化
		m_field = std::make_unique<field>();
		m_field->setdepth(depth);
		m_field->setwidth(width);
		m_field->setdividex(dividex);
		m_field->setdividez(dividez);

		m_field->init();
	}

	ImGui::End();
}

// フィールドに凸凹にする
void CameraBasicScene::debugFieldUnduration() {

	ImGui::Begin("debug Field Remake with unduration");

	static float minheight = 0.0f;
	static float maxheight = 100.0f;

	ImGui::SliderFloat("low height", &minheight, 0.0f, 10.0f);
	ImGui::SliderFloat("max hight", &maxheight, 0.0f, 100.0f);

	static float perlinscale = 0.5f;		// ノイズの細かさ（お好みで 0.02～0.2 くらい）
	static float perlinoffsetX = 10.0f;		// シード代わりのオフセット（任意）
	static float perlinoffsetZ = 10.0f;

	ImGui::SliderFloat("perlin scale", &perlinscale, 0.0f, 5.0f);
	ImGui::SliderFloat("perlin offset x", &perlinoffsetX, 10.0f, 800.0f);
	ImGui::SliderFloat("perlin offset z", &perlinoffsetZ, 10.0f, 800.0f);

	if (ImGui::Button("remake field with random")) {
		m_field->makeundulationwithrandom(minheight, minheight + maxheight);
	}
	if (ImGui::Button("remake field with perlin")) {

		m_field->makeundulationwithperlin(minheight, minheight + maxheight,
			perlinscale, perlinoffsetX, perlinoffsetZ);
	}

	ImGui::End();

}

/**
 * @brief シーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void CameraBasicScene::update(uint64_t deltatime)
{
	m_camera->Draw();

	// 障害物
	for (auto& obs : m_obstacles) {
		SRT srt = obs->getSRT();
		// 敵の現在位置の高さを取得
		float height = m_field->GetHeight2(srt.pos);
		srt.pos.y = height;

		obs->setSRT(srt);
	}

	// 壁
	for (auto& wall : m_walls) {
		SRT srt = wall->getSRT();
		// 敵の現在位置の高さを取得
		float height = m_field->GetHeight2(srt.pos);
		srt.pos.y = height;

		wall->setSRT(srt);
		wall->makeplanedata();
	}

	// 敵
	for (auto& enemy : m_enemies) {
		enemy->update(deltatime);
	}

	// TOWER
	m_tower->update(deltatime);

	// プレイヤ更新
	m_player->update(deltatime);
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void CameraBasicScene::draw(uint64_t deltatime)
{

	m_camera->Draw();

	// フィールド描画
	m_field->draw(deltatime);

	// タワー描画
//	m_tower->draw(deltatime);

	// 敵描画
//	for (auto& e : m_enemies) {
//		e->draw(deltatime);
//	}

	// プレイヤを描画
	m_player->draw(deltatime);

	// 障害物
	for (auto& obs : m_obstacles) {
		obs->draw(deltatime);
	}

	// 壁
	for (auto& wall : m_walls) {
		wall->draw(deltatime);
	}

	// aimをビルボードで描画
	SRT srt = m_tower->getSRT();
	m_aim->setPosition(srt.pos);
	m_aim->drawbillboard(m_camera.get());
}

/**
 * @brief シーンの初期化処理
 */
void CameraBasicScene::init()
{
	// カメラ(3D)の初期化
	m_camera = std::make_unique <Camera>(Vector3(0,0,-100),Vector3(0,0,0),Vector3(0,1,0));

	// DirectWrite生成
	m_directwrite = std::make_unique<DirectWrite>(&m_fontdata);
	m_directwrite->Init(Renderer::GetSwapChain());

	// リソースを読み込む
	resourceLoader();

	// プレイヤ
	m_player = std::make_unique<player>(this);
	m_player->init();

	// aim初期化
	m_aim = std::make_unique<aim>();
	m_aim->init();

	// フィールド初期化
	m_field = std::make_unique<field>();
	m_field->init();

	// タワー初期化
	m_tower = std::make_unique<tower>(this);
	m_tower->init();

	// 乱数エンジンを初期化
	auto& rng = RandomEngine::tls();

	rng.uniformReal(-500, 500);

	// 敵初期化
	for (int cnt = 0; cnt < ENEMYMAX; cnt++) {
		m_enemies[cnt] = std::make_unique<enemy>(this);
		m_enemies[cnt]->init();
		SRT srt = m_enemies[cnt]->getSRT();

		double scale = rng.normal(100, 80);
		scale  = scale / 300.0;
	
		srt.scale = Vector3(
			static_cast<float>(scale),
			static_cast<float>(scale),
			static_cast<float>(scale));

		srt.pos = Vector3(
			static_cast<float>(rng.uniformReal(-500.0f, 500.0f)),
			0,
			static_cast<float>(rng.uniformReal(-500.0f, 500.0f)));

		m_enemies[cnt]->setSRT(srt);
	}

	// 障害物群初期化
	{
		// 乱数エンジンを初期化
		auto& rng = RandomEngine::tls();

		rng.uniformReal(-500, 500);

		// 障害物初期化
		for (int cnt = 0; cnt < OBSTACLEMAX; cnt++) {
			m_obstacles[cnt] = std::make_unique<obstacle>(this);
			m_obstacles[cnt]->init();

			SRT srt;
			srt.scale = Vector3(
				static_cast<float>(rng.uniformReal(10.0f, 30.0f)),
				static_cast<float>(rng.uniformReal(10.0f, 30.0f)),
				static_cast<float>(rng.uniformReal(10.0f, 30.0f)));

			srt.rot = Vector3(
				0,
				static_cast<float>(rng.uniformReal(-PI, PI)),
				0);

			srt.pos = Vector3(
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)),
				0,
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)));

			// 障害物高さを取得
			float height = m_field->GetHeight2(srt.pos);
			srt.pos.y = height;

			m_obstacles[cnt]->setSRT(srt);
		}

	}

	// 壁群初期化
	{
		// 乱数エンジンを初期化
		auto& rng = RandomEngine::tls();

		// 障害物初期化
		for (int cnt = 0; cnt < WALLMAX; cnt++) {

			// 配置情報作成
			SRT srt{};
			srt.rot = Vector3(
				0,
				static_cast<float>(rng.uniformReal(-PI, PI)),
				0);

			srt.pos = Vector3(
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)),
				0,
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)));

			// 地面の高を取得
			float height = m_field->GetHeight2(srt.pos);
			srt.pos.y = height;

			// 壁の高さと幅を決定
			float h = 100.0f;
			float w = static_cast<float>(rng.uniformReal(50.0f, 300.0f));

			// 壁生成
			m_walls[cnt] = std::make_unique<wall>(this,srt,w,h);
			m_walls[cnt]->init();
		}
	}

	DebugUI::RedistDebugFunction([this]() {
		debugUICamera();
		});

	DebugUI::RedistDebugFunction([this]() {
		debugFieldRemake();
		});

	DebugUI::RedistDebugFunction([this]() {
		debugFieldUnduration();
		});

	DebugUI::RedistDebugFunction([this]() {
		debugDirectionalLight();
		});
}

/**
 * @brief シーンの終了処理
 */
void CameraBasicScene::dispose()
{
}

/**
 * @brief リソースを読み込む
 */
void CameraBasicScene::resourceLoader()
{
	{
		std::unique_ptr<CShader> shader = std::make_unique<CShader>();
		shader->Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");
		MeshManager::RegisterShader<CShader>("oneskinshader", std::move(shader));
	}

	{
		// 光源計算ありシェーダー
		std::unique_ptr<CShader> shader = std::make_unique<CShader>();
		shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");
		MeshManager::RegisterShader<CShader>("lightshader", std::move(shader));
	}

	{
		// 光源計算なしシェーダー
		std::unique_ptr<CShader> shader = std::make_unique<CShader>();
		shader->Create("shader/unlitTextureVS.hlsl", "shader/unlitTexturePS.hlsl");
		MeshManager::RegisterShader<CShader>("unlightshader", std::move(shader));
	}

	{
		// 光源計算あり（スペキュラ計算）シェーダー
		std::unique_ptr<CShader> shader = std::make_unique<CShader>();
		shader->Create(
			"shader/vertexLightingwithSpecularVS.hlsl",
			"shader/vertexLightingwithSpecularPS.hlsl");
		MeshManager::RegisterShader<CShader>("lightshaderSpecular", std::move(shader));
	}

	{
		// 光源計算あり（スペキュラ計算）シェーダー
		std::unique_ptr<CShader> shader = std::make_unique<CShader>();
		shader->Create(
			"shader/vertexLightingwithSpecular2VS.hlsl",
			"shader/vertexLightingwithSpecular2PS.hlsl");
		MeshManager::RegisterShader<CShader>("lightshaderSpecular2", std::move(shader));
	}

	// メッシュデータ読み込み（敵用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/car001.x", "assets/model/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("car001.x", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("car001.x", std::move(srenderer));
	}

	// メッシュデータ読み込み（プレイヤ用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/car000.x", "assets/model/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("car000.x", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("car000.x", std::move(srenderer));
	}

	// メッシュデータ読み込み（障害物用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/obj/box.obj", "assets/model/obj/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("obstaclebox", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclebox", std::move(srenderer));
	}

	// メッシュデータ読み込み（タワー）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/tower/Only Tower.obj", "assets/model/tower/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("Tower", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Tower", std::move(srenderer));
	}

	// メッシュデータ読み込み（障害物用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/obj/box.obj", "assets/model/obj/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("obstaclebox", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclebox", std::move(srenderer));
	}
}