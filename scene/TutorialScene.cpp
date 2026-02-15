#include "TutorialScene.h"
#include "../system/CDirectInput.h"
#include "../utility/ScreenToWorld.h"
#include "../system/SphereDrawer.h"
#include "../system/DebugUI.h"
#include "../system/CPolar3D.h"
#include "../system/meshmanager.h"
#include "../system/AimOrientation.h"
#include "../system/camera.h"
#include "../utility/spline.h"
#include "../utility/MathUtil.h"
#include "../system/EnemyCounter.h"
#include "../system/collision.h"
#include <iostream>
#include "../system/scenemanager.h"
#include "../sound.h"
#include "../gameobject/resource.h"
#include "../gameobject/boxcreate.h"
#include <xinput.h>
#include <cstring>
#pragma comment(lib, "xinput.lib")

namespace {

	// Door座標
	constexpr Vector3 DOOR_POS = Vector3(0.0f, 0.0f, 880.0f);

	// クランプ範囲
	constexpr float PLAY_AREA_HALF_WIDTH_X = 240.0f;  // X
	constexpr float PLAY_AREA_BACK_Z = -800.0f;       // Y
	constexpr float PLAY_AREA_FRONT_MARGIN_Z = 20.0f; // ドア手前の余白

	inline float ClampFloat(float v, float lo, float hi)
	{
		if (v < lo) return lo;
		if (v > hi) return hi;
		return v;
	}

	inline void ClampPosZ(Vector3& pos)
	{

		// Z: 奥(負)～ドア手前までを制限
		const float minZ = PLAY_AREA_BACK_Z;
		const float maxZ = DOOR_POS.z - PLAY_AREA_FRONT_MARGIN_Z;

		pos.z = ClampFloat(pos.z, minZ, maxZ);
	}

	constexpr float NO_SUPPORT_Y = -1.0e9f;

	// Xゲームパットのトリガー検出
	inline bool PadTrigger()
	{
		static bool s_prevX = false;
		static bool s_prevB = false;

		XINPUT_STATE st{};
		std::memset(&st, 0, sizeof(st));
		if (XInputGetState(0, &st) != ERROR_SUCCESS)
		{
			s_prevX = false;
			s_prevB = false;
			return false;
		}

		const bool nowX = (st.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
		const bool nowB = (st.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;

		const bool trgX = (nowX && !s_prevX);
		const bool trgB = (nowB && !s_prevB);

		s_prevX = nowX;
		s_prevB = nowB;

		return trgX || trgB;
	}

}

/**
 * @brief コンストラクタ
 */
TutorialScene::TutorialScene()
{
}

/**
 * @brief カメラの設定
 *
 */
void TutorialScene::debugUICamera() {

	static float azimuth = 0.0f;
	static float elevation = 0.0f;
	static float radius = 1000.0f;

	ImGui::Begin("debug Camera");

	//視点の位置
	Vector3 camerapos = m_camera->GetPosition();
	//注視点
	Vector3 lookatpos = m_player->getSRT().pos;// m_camera->GetLookat();
	//上ベクトル
	Vector3 up = m_camera->GetUP();

	ImGui::SliderFloat3("camera pos ", &camerapos.x, -PI, PI);
	ImGui::SliderFloat3("camera up  ", &up.x, -PI, PI);

	ImGui::Separator();
	ImGui::Separator();

	ImGui::SliderFloat("azimuth ", &azimuth, -PI, PI);
	ImGui::SliderFloat("elevation ", &elevation, -PI / 2.0f, PI / 2.0f);
	ImGui::SliderFloat("radius ", &radius, 100.0f, 1000.0f);

	CPolor3D polor(radius, elevation, azimuth);
	Vector3 cpos = polor.ToCartesian();

	CPolor3D polorup(1.0f, elevation + PI / 2.0f, azimuth);
	Vector3 upvector = polorup.ToCartesian();

	// 正規化（簡易）
	float l = sqrtf(upvector.x * upvector.x + upvector.y * upvector.y + upvector.z * upvector.z);
	if (l > 1e-6f) {
		upvector.x /= l; upvector.y /= l; upvector.z /= l;
	}
	else {
		upvector = Vector3(0, 1, 0);
	}

	Vector3 viewDir = lookatpos - (cpos + lookatpos);
	// 正規化 viewDir
	float vl = sqrtf(viewDir.x * viewDir.x + viewDir.y * viewDir.y + viewDir.z * viewDir.z);
	if (vl > 1e-6f) {
		viewDir.x /= vl; viewDir.y /= vl; viewDir.z /= vl;
	}
	float dot = fabsf(viewDir.x * upvector.x + viewDir.y * upvector.y + viewDir.z * upvector.z);
	if (dot > 0.999f) {
		upvector = Vector3(0, 1, 0);
	}

	m_camera->SetPosition(cpos + lookatpos);
	m_camera->SetLookat(lookatpos);
	m_camera->SetUP(upvector);

	ImGui::Separator();
	ImGui::Separator();
	ImGui::SliderFloat3("nearpoint ", &m_nearpoint.x, -10000.0f, 10000.0f);
	ImGui::SliderFloat3("farpoint ", &m_farpoint.x, -10000.0f, 10000.0f);

	ImGui::Separator();
	ImGui::Separator();
	ImGui::SliderFloat3("pickuppos ", &m_pickuppos.x, -10000.0f, 10000.0f);

	ImGui::End();
}

// 球と円柱の当たり判定
bool TutorialScene::CollisionSphereCylinder_Push(
	const Call::Collision::BoundingSphere& sphere,
	const Call::Collision::BoundingCylinder& cylinder,
	Vector3& outPush)
{
	outPush = Vector3(0, 0, 0);

	Vector3 axis = cylinder.top - cylinder.bottom;
	const float axisLenSq = axis.LengthSquared();
	if (axisLenSq <= 1e-8f) return false;

	const Vector3 v = sphere.center - cylinder.bottom;
	float t = v.Dot(axis) / axisLenSq;
	if (t < 0.0f) t = 0.0f;
	else if (t > 1.0f) t = 1.0f;

	const Vector3 closest = cylinder.bottom + axis * t;

	Vector3 diff = sphere.center - closest;

	float distSq = diff.LengthSquared();
	const float r = sphere.radius + cylinder.radius;

	if (distSq >= r * r) return false;

	 //負になったら0に潰すNaN防止
	if (distSq < 0.0f) distSq = 0.0f;

	const float dist = sqrtf(distSq);

	if (dist <= 1e-6f)
	{
		outPush = Vector3(r, 0.0f, 0.0f);
		return true;
	}

	diff /= dist;
	outPush = diff * (r - dist);
	return true;
}

bool TutorialScene::ResolveEnemyTowerCollision(enemy& e, const tower& t)
{
	// 敵の球
	Call::Collision::BoundingSphere s{};
	s.center = e.getSRT().pos;
	s.radius = e.GetCollisionRadius();

	Vector3 push;
	if (CollisionSphereCylinder_Push(s, t.GetCylinder(), push))
	{
		// 押し戻し
		auto srt = e.getSRT();
		srt.pos += push;
		e.setSRT(srt);

		// EnemyStopとかの移動止めあったほうがいいかも

		return true;
	}
	return false;
}

// プレイヤとリソースの当たり判定
static bool PlayerResourceCollision(const player& p, const Resource& r)
{
	Call::Collision::BoundingSphere bsP{ p.GetColliderCenter(), p.GetCollisionRadius() };
	Call::Collision::BoundingSphere bsR{ r.GetColliderCenter(), r.GetCollisionRadius() };
	return CollisionSphere(bsP, bsR);
}

// 弾と敵の当たり判定
static bool BulletEnemyCollision(PlayerBullet& b, enemy& e)
{

	Call::Collision::BoundingSphere bsBullet{ b.GetColliderCenter(), b.GetCollisionRadius()};
	Call::Collision::BoundingSphere bsEnemy{e.GetColliderCenter(),e.GetCollisionRadius()};
	const bool hit = Call::Collision::CollisionSphere(bsBullet, bsEnemy);
	if (hit) GetXAud()->soundSEPlay((int)SoundSEAssets::SeDmg);
	return hit;
}

// プレイヤと敵の当たり判定
static bool PlayerEnemyCollision(player& p, enemy& e)
{
	Call::Collision::BoundingSphere bsPlayer{
		p.GetColliderCenter(),
		p.GetCollisionRadius()
	};

	Call::Collision::BoundingSphere bsEnemy{
		e.GetColliderCenter(),
		e.GetCollisionRadius()
	};
	return Call::Collision::CollisionSphere(bsPlayer, bsEnemy);
}

//爆発によるドロップ倍率
float ExplosionDropMul(int killCount)
{
	if (killCount >= 3) return 2.0f;//元5体
	if (killCount >= 2) return 1.5f;//元3体
	return 1.0f;
}

//当りを回す必要があるか？
void TutorialScene::CollisionStep()
{
	if (!m_player) return;
	if (!m_tower) return;

	//resource x player
	for (auto& re : m_resource)
	{
		if (!re || !re->GetResourceLife()) continue;

		if (PlayerResourceCollision(*m_player, *re))
		{
			std::cout << "[RES HIT] add=" << re->GetValue()
				<< " mp(before)=" << m_player->GetMp() << "\n";

			m_player->AddMp(re->GetValue());

			std::cout << "[RES HIT] mp(after)=" << m_player->GetMp() << "\n";

			re->Kill();
		}
	}

	m_tower->Reboot();		// タワー接触ON

	if (EnemyCounter::GetInstance().GetAliveCount() <= 0) return;

	// 生存弾・生存敵がいるか
	const bool hasAliveBullet = AnyAlive(
		m_playerBullets,[](const PlayerBullet& b) { return b.GetisAlive(); });

	// 敵がいるか
	const bool hasAliveEnemy = AnyAlive(
		m_enemies,[](const enemy& e) { return e.IsAlive(); });

	// 保険
	if (!hasAliveEnemy) return;

	// 弾x敵
	if (hasAliveBullet)
	{
		for (auto& pb : m_playerBullets)
		{
			if (!pb || !pb->GetisAlive()) continue;

			for (auto& e : m_enemies)
			{
				if (!e || !e->IsAlive()) continue;
				if (!e->ShouldCollideWith(pb->GetNo())) continue;// Ghostの場合3以外場合除外

				// Trap
				if (pb->IsTrap())
				{
					const float d = Math::Distance(
						pb->GetExplosionCenter(),
						e->GetColliderCenter()
					);
					const float thr =
						pb->GetCollisionRadius() + e->GetCollisionRadius();
					if (d > thr) continue;

					m_hasExplosion = true;
					m_exCenter = pb->GetExplosionCenter();
					m_exRadius = pb->GetExplosionRadius();
					m_exDamage = pb->GetDamage();

					const Vector3 exPos = pb->GetColliderCenter();
					m_effectExplosion->StartExplosion(exPos);

					pb->Explode();
					break; // この弾は終わり
				}

				// Shot
				if (!BulletEnemyCollision(*pb, *e))
					continue;

				e->m_hitFromExplosion = false;
				e->OnHitBullet(pb->GetNo(), pb->GetDamage());
				pb->Kill();
				break;
			}
		}
	}

	// 爆発ダメージ処理
	if (m_hasExplosion)
	{
		int killCount = 0;

		// まずダメージ＆killCount
		for (auto& e : m_enemies)
		{
			if (!e) continue;
			if (Math::Distance(m_exCenter, e->GetColliderCenter())
			> (m_exRadius + e->GetCollisionRadius()))
				continue;

			const bool wasAlive = e->IsAlive();
			e->SetHitFromExplosion(true);
			e->Damage(m_exDamage);

			if (wasAlive && !e->IsAlive()) killCount++;
		}

		// ここで倍率確定
		const float mul = ExplosionDropMul(killCount);

		// 巻き込まれた敵に保存
		if (mul > 1.0f)
		{
			for (auto& e : m_enemies)
			{
				if (!e) continue;
				if (Math::Distance(m_exCenter, e->GetColliderCenter())
				> (m_exRadius + e->GetCollisionRadius()))
					continue;

				e->SetExplosionDropMul(mul);
			}
		}

		m_hasExplosion = false;
	}

	// プレイヤx敵
	for (auto& e : m_enemies)
	{
		if (!e || !e->IsAlive()) continue;

	if (!PlayerEnemyCollision(*m_player, *e))
		continue;

	if (m_player->Damage(1))
	{
		m_player->OnDamage();
		m_player->Knockback(e->getSRT().pos);
		std::cout << "Player Damaged" << std::endl;
		break;
	}
	}

	//enemy x タワー

	for (auto& ep : m_enemies)
	{
		if (!ep) continue;
		if (!ep->IsAlive()) continue;

		if (ResolveEnemyTowerCollision(*ep, *m_tower))
		{
			auto now = Time::Get().Now();
			static constexpr auto CD = Time::Ms(500); // 0.5秒

			if (ep->CanHitTower(now))
			{
				XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::Towerdm);

				if (m_tower->UnderAtack(ep->GetAttackPower()))
				{
					ep->SetNextTowerHit(now + CD);
				}
			}
		}
	}
}

// ステージスクリプトの切り替え部分
void TutorialScene::SetupStage(Stage stage)
{
	m_stage = stage;

	switch (stage)
	{
	case Stage::Stage1:
		m_script = STAGE1_SCRIPT;
		m_scriptCount = std::size(STAGE1_SCRIPT);
		break;

	case Stage::Stage2:
		m_script = STAGE2_SCRIPT;
		m_scriptCount = std::size(STAGE2_SCRIPT);
		break;
	case Stage::Stage3:
			m_script = STAGE3_SCRIPT;
			m_scriptCount = std::size(STAGE3_SCRIPT);
			break;
	case Stage::Stage4:
		m_script = STAGE4_SCRIPT;
		m_scriptCount = std::size(STAGE4_SCRIPT);
		break;
	case Stage::Stage5:
		m_script = STAGE5_SCRIPT;
		m_scriptCount = std::size(STAGE5_SCRIPT);
		break;
	}

	m_stageStartTime = Time::Get().Now();
	m_scriptIndex = 0;
	m_elapsedMs = 0;
	m_nextSpawnPoint = 0;

	// Wave総数(分母)を入れる
	int total = 0;
	for (size_t i = 0; i < m_scriptCount; ++i)
		total += m_script[i].count;

	EnemyCounter::GetInstance().StartWave(total);

	// チュートリアル初回だけ
	if (!m_givenTutorialMp && stage == Stage::Stage1 && m_player)
	{
		m_player->AddMp(10);
		m_givenTutorialMp = true;
	}

	//m_killWindow.Reset();
}

void TutorialScene::StartNextWave()
{
	// Clear解除
	m_cleared = false;
	Clear();
	m_uiState = UiState::InGame;
	m_clearTimerMs = 0;

	const int next = (static_cast<int>(m_stage) + 1) % static_cast<int>(Stage::Count);
	SetupStage(static_cast<Stage>(next));

}

//描画用ステージ番号取得
int TutorialScene::GetStageIndex() const
{
	return static_cast<int>(m_stage) + 1;
}

//お掃除
void TutorialScene::Clear()
{
	for (auto& e : m_enemies)
	{
		if (!e) continue;
		e->SetSpawned(false);
	}

	// 設置玉（Trap）全消し 何かスキルとかルーレット罰とかで使えるなら残す
	//for (auto& pb : m_playerBullets)
	//{
	//	if (!pb) continue;
	//	if (pb->GetType() == PlayerBullet::BulletType::Trap)
	//	{
	//		pb->Kill();   // or ForceReset / SetAlive(false)
	//	}
	//}
	// ロボ用に死体リストを持つなら
	//m_corpses.clear();
}

// リソーススポーン
void TutorialScene::SpawnResource(const Vector3& pos)
{
	for (auto& re : m_resource)
	{
		if (!re) continue;

		if (!re->GetResourceLife())
		{
			re->Spawn(pos, 1, true);
			return;
		}
	}
}

// 弾箱ソート
PlayerBullet* TutorialScene::TryReserveBullet()
{
	// 空いている箱を先頭から探す
	for (auto& pb : m_playerBullets)
	{
		if (!pb) continue;

		if (!pb->GetisAlive())
		{
			return pb.get();
		}
	}
	return nullptr;
}

void TutorialScene::GameOver() {

	if (m_isGameOver) return;   // 多重防止
	m_isGameOver = true;
	m_goCircleScale = 0.0f;              // 小から
	m_goFadeStart = Time::Get().Now();
	XAudSound::GetInstance()->soundBGMStop();

}

void TutorialScene::update(uint64_t deltatime)
{
	m_camera->GetViewMatrix();
	//m_camera->Draw();
	m_player->update(deltatime);

	if (m_player)
		UI::Get().Update(deltatime, m_player->GetHP(), m_player->GetHpMax());

	// HP0でGameOver確定（returnしない）
	if (!m_isGameOver && m_player && m_player->GetHP() <= 0)
		GameOver();

	// 落下死
	if (!m_isGameOver && m_player && m_player->getSRT().pos.y < FALL_Y)
		GameOver();

	// タワー死
	if (!m_isGameOver && m_tower && m_tower->IsDead())
		GameOver();

	// ゲームオーバー処理
	if (m_isGameOver)
	{
		const uint64_t elapsed = Time::ElapsedMs(m_goFadeStart);

		float t = (float)elapsed / 600.0f;
		if (t > 1.0f) t = 1.0f;
		m_goCircleScale = 8.0f * t;

		// 死んだ時だけHPバー0待ちをする
		const bool isHpDeath =
			(m_player && m_player->GetHP() <= 0) &&
			!(m_tower && m_tower->IsDead()) &&
			!(m_player && m_player->getSRT().pos.y < FALL_Y);

		const bool timeOK = (elapsed >= 600 + GO_WAIT_MS);
		const bool hpOK = (!isHpDeath) || UI::Get().IsHpVisualEmpty();

		if (timeOK && hpOK)
			SceneManager::SetCurrentScene("GameOverScene");

		return;
	}

	// ショット処理
	if (m_player->ConsumeShotFlag())
	{
		const BulletGimmick::BulletNo no = m_player->GetSelectedNo();
		const auto& sp = BulletGimmick::Spec(no);

		// MP消費
		if (sp.mpCost > 0)
		{
			if (!m_player->ConsumeMp(sp.mpCost)) return;
		}

		PlayerBullet* pb = TryReserveBullet();
		if (!pb) return;

		const Vector3 spawnPos = m_player->getSRT().pos;
		pb->Spawn(spawnPos, m_player->GetPlayerDir(), no);

		std::cout << "Spawn Bullet actorNo=" << static_cast<int>(pb->GetActorNo()) << std::endl;
	}

	//カメラ
	if (!m_camInit)
	{
		m_camPos = m_camera->GetPosition(); // 最初の1回だけ同期
		m_camInit = true;
	}

	const Vector3 playerPos = m_player->getSRT().pos;

	// 目標位置
	Vector3 targetPos;
	targetPos.x = playerPos.x;
	targetPos.y = playerPos.y + 80.0f;
	targetPos.z = playerPos.z - 150.0f;

	// Y追従
	float follow = 0.08f;
	m_camPos.y += (targetPos.y - m_camPos.y) * follow;

	m_camPos.x = targetPos.x;
	m_camPos.z = targetPos.z;

	// 注視点
	Vector3 vPlayerDir = m_player->GetPlayerDir();
	Vector3 lookatpos = playerPos;
	lookatpos.y = m_camPos.y - 50.0f; // 既存の高さ合わせは維持

	m_camera->SetPosition(m_camPos);
	m_camera->SetLookat(lookatpos);
	m_camera->SetUP(Vector3(0, 1, 0));

	// スポナ更新
	const uint64_t spawnDt = (deltatime > 50) ? 50 : deltatime;

	for (auto& pb : m_playerBullets) {

		if (pb) {
			pb->update(deltatime);
		}
	}

	for (auto& e : m_enemies) {
		if (e) {
			e->update(deltatime);
		}
	}

	// 当たり判定
	CollisionStep();

	if (m_player)
	{
		auto srt = m_player->getSRT();
		ClampPosZ(srt.pos);

		m_player->setSRT(srt);
	}

	for (auto& e : m_enemies)
	{
		if (!e || !e->IsAlive()) continue;

		auto srt = e->getSRT();
		ClampPosZ(srt.pos);
		e->setSRT(srt);
	}

	// ステージスクリプト処理
	uint64_t elapsedMs =
		Time::ElapsedMs(m_stageStartTime);

	while (m_scriptIndex < m_scriptCount &&
		m_script[m_scriptIndex].timeMs <= elapsedMs)
	{
		const auto& s = m_script[m_scriptIndex];
		const Vector3 pos = spawnpoints::SPAWN_ALL[s.pointIndex];

		for (int i = 0; i < s.count; ++i)
		{
			for (auto& e : m_enemies)
			{
				if (e && !e->IsAlive())
				{
					// エネミータイプをセット
					e->SetEnemyState(s.state);
					e->Spawn(pos);
					break;
				}
			}
		}
		++m_scriptIndex;
	}

	// リソース更新
	for (auto& re : m_resource)
	{
		if (re && re->GetResourceLife()) re->update(deltatime);
	}

	// TOWER
	m_tower->update(deltatime);
	m_tower->Reboot(); // タワー接触フレームON

	if (m_effectExplosion)
		m_effectExplosion->update(deltatime);

	if (!m_cleared && EnemyCounter::GetInstance().IsWaveCleared())
	{
		m_cleared = true;
		m_uiState = UiState::Clear;
		m_clearTimerMs = 0;
		m_clearAccepted = false;   // 悪さするバッファ対策用
		return;
	}

	if (m_uiState == UiState::Clear)
	{
		m_clearTimerMs += deltatime;

		// 拾い物だけは回す
		for (auto& re : m_resource)
		{
			if (!re || !re->GetResourceLife()) continue;

			re->update(deltatime);

			// 取得判定:球
			if (PlayerResourceCollision(*m_player, *re))
			{

			}
		}

		const bool accept = (m_clearTimerMs > 500);

		// Enter or PadX or PadBで次へ
		auto& di = CDirectInput::GetInstance();
		const bool enterTrig =
			di.CheckKeyBufferTrigger(DIK_RETURN) ||
			di.CheckKeyBufferTrigger(DIK_NUMPADENTER);

		const bool nextTrig = enterTrig || PadTrigger();

		if (accept && nextTrig)
		{
			m_clearAccepted = true;

			for (auto& e : m_enemies)
				if (e) e->SetSpawned(false);

			StartNextWave();
			m_uiState = UiState::InGame;
			m_cleared = false;
		}
		return;
	}
}

void TutorialScene::draw(uint64_t deltatime)
{
	m_camera->Draw();
	CShader* shader = MeshManager::getShader<CShader>("lightshader");
	float rad = DirectX::XM_PI / 180.0f;

	// 背景モデル
	{
		SRT srt;
		srt.pos = Vector3(1, -200, 1);
		srt.scale = Vector3(8.0f, 8.0f, 8.0f);
		srt.rot = Vector3(0, PI, 0);

		CStaticMeshRenderer* sr = MeshManager::getRenderer<CStaticMeshRenderer>("Tower");

		Matrix4x4 mtx = srt.GetMatrix();
		Renderer::SetWorldMatrix(&mtx);
		shader->SetGPU();
		sr->Draw();
	}
	{
		SRT srt;
		srt.pos = Vector3(0, 0, 880);
		srt.scale = Vector3(0.8f, 0.8f, 0.8f);
		srt.rot = Vector3(0, 0, 0);

		CStaticMeshRenderer* sr = MeshManager::getRenderer<CStaticMeshRenderer>("Door.fbx");

		Matrix4x4 mtx = srt.GetMatrix();
		Renderer::SetWorldMatrix(&mtx);
		shader->SetGPU();
		sr->Draw();
	}

	{
		auto* sr = MeshManager::getRenderer<CStaticMeshRenderer>("WallEntrance.fbx");
		shader->SetGPU();

		const float rad = DirectX::XM_PI / 180.0f;

		auto DrawWall = [&](float x, float y, float z, float yawDeg, float scale)
			{
				SRT srt;
				srt.pos = Vector3(x, y, z);
				srt.scale = Vector3(scale, scale, scale);
				srt.rot = Vector3(0, yawDeg * rad, 0);

				Matrix4x4 mtx = srt.GetMatrix();
				Renderer::SetWorldMatrix(&mtx);
				sr->Draw();
			};

		// 中央手動補完
		const float heightY = -230.0f;
		const float xLeft = -240.0f;
		const float xRight = 240.0f;

		DrawWall(xRight, heightY, 80.0f, 90.0f, 0.9f);
		DrawWall(xLeft, heightY, 80.0f, 90.0f, 0.9f);

		// 自動配置
		const float zStart = -800.0f;
		const float zRange = 1600.0f;

		const float modelDepthZ = 200.0f;
		float gapZ = -25.0f;
		const float pitchZ = modelDepthZ + gapZ;

		const float centerDeadZoneHalf = 100.0f;

		for (float offset = 0.0f; offset <= zRange + 0.0001f; offset += pitchZ)
		{
			float z = zStart + offset;
			if (fabsf(z) < centerDeadZoneHalf) continue;

			DrawWall(xLeft, heightY, z, 90.0f, 0.9f);
			DrawWall(xRight, heightY, z, 90.0f, 0.9f);
		}
	}

	// フィールド描画
	m_field->draw(deltatime);

	//影描画
	Renderer::SetDepthEnable(false);
	Renderer::SetBlendState(BS_SUBTRACTION);

	//enemy
	for (auto& e : m_enemies)
	{
		if (!e || !e->IsAlive()) continue;

		const Vector3 p = e->getSRT().pos;
		const float groundY = m_field->GetHeight2(p) + 0.1f; // 少し浮かす
		const float r = e->GetCollisionRadius();

		m_blobshadow->Draw(r, groundY, p.x, p.z);
	}

	//bullet
	for (auto& pb : m_playerBullets)
	{
		if (!pb || !pb->GetisAlive()) continue;

		const Vector3 p = pb->getSRT().pos;
		const float groundY = m_field->GetHeight2(p) + 0.05f; // 少し浮かす
		const float r = pb->GetCollisionRadius();

		m_blobshadow->Draw(r, groundY, p.x, p.z);
	}

	Renderer::SetBlendState(BS_ALPHABLEND);
	Renderer::SetDepthEnable(true);

	//敵描画
	for (auto& e : m_enemies) {
		if (!e) continue;
		e->draw(deltatime);
	}

	// プレイヤを描画
	m_player->draw(deltatime);
	
	//Bullet描画
	for (auto& pb : m_playerBullets) {
		pb->draw(deltatime);
	}

	// エフェクト描画
	if (m_effectExplosion)
		m_effectExplosion->draw(deltatime);

	// タワー影描画
	{
		const Vector3 t = m_tower->getSRT().pos;
		const float groundY = m_field->GetHeight2(t);

		SphereDrawerDrawShadow(
			13.5f,
			groundY,
			t.x, t.z,
			Color(0.0f, 0.0f, 0.0f, 0.5f)
		);
	}

	// タワー描画
	m_tower->draw(deltatime);

	// リソース描画
	Renderer::SetDepthEnable(false);
	for (auto& re : m_resource)
	{
		if (re && re->GetResourceLife())
			re->drawbillboard(m_camera.get());
	}
	Renderer::SetDepthEnable(true);

	// ノード描画
	for (auto& node : m_nodes) {
		SphereDrawerDraw(10.0f, Color(1, 0, 0, 1), node.pos.x, node.pos.y, node.pos.z);
	}

	// プレイヤー影描画
	{
		const Vector3 p = m_player->getSRT().pos;
		const float groundY = m_field->GetHeight2(p);

		SphereDrawerDrawShadow(
			12.0f,
			groundY,
			p.x, p.z,
			Color(0.0f, 0.0f, 0.0f, 0.50f)
		);
	}

	//残数
	FontData prev = m_fontdata;

	// 小さめフォント
	FontData smallFont = prev;
	smallFont.fontSize = 18.0f;
	m_directwrite->SetFont(smallFont);

	const int total = EnemyCounter::GetInstance().GetWaveTotal();
	const int killed = EnemyCounter::GetInstance().GetWaveKilled();
	int left = total - killed;
	if (left < 0) left = 0;

	const std::string text = "残り: " + std::to_string(left) + "体";

	m_directwrite->DrawString(
		text.c_str(),
		D2D1_RECT_F{ 1150, 70, 1400, 90 },
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		false
	);

	// 戻す
	m_directwrite->SetFont(prev);

	if (m_circle && m_goCircleScale > 0.0f)
	{
		const float cx = Application::GetWidth() * 0.5f;
		const float cy = Application::GetHeight() * 0.5f;

		const Vector3 rot(0, 0, 0);
		const Vector3 s(m_goCircleScale, m_goCircleScale, 1.0f);

		// 画面中央に円を拡大描画
		m_circle->Draw(s, rot, Vector3(cx, cy, 0));
	}

	//ClearUI
	if (m_uiState == UiState::Clear)
	{
		const float cx = Application::GetWidth() * 0.5f;
		const float cy = Application::GetHeight() * 0.5f;

		// 文字（DirectWrite）}
		m_directwrite->DrawString(
			"CLEAR!",
			D2D1_RECT_F{ cx - 300, cy - 180, cx + 300, cy - 80 },
			D2D1_DRAW_TEXT_OPTIONS_NONE,
			true // 中央寄せがあるなら
		);

		if (m_clearTimerMs > 500)
		{
			m_directwrite->DrawString(
				"ENTER: NEXT",
				D2D1_RECT_F{ cx - 400, cy - 60, cx + 400, cy + 40 },
				D2D1_DRAW_TEXT_OPTIONS_NONE,
				true
			);
		}
	}

	UI::Get().UpdateEnemyGauge(
		EnemyCounter::GetInstance().GetWaveKilled(),
		EnemyCounter::GetInstance().GetWaveTotal());

	UI::Get().SetTrapMode(
		m_player ? BulletGimmick::Spec(m_player->GetSelectedNo()).isTrap : false
	);
	UI::Get().SetStageIndex(GetStageIndex());
	UI::Get().Draw();
	UI::Get().SetMp(m_player ? m_player->GetMp() : 0);
	UI::Get().SetCurrentBulletNo(m_player->GetSelectedNo());
}

/**
 * @brief シーンの初期化処理
 */
void TutorialScene::init()
{
	// カメラ(3D)の初期化
	m_camera = std::make_unique <Camera>(Vector3(0, 0, -100), Vector3(0, 0, 0), Vector3(0, 1, 0));

	// DirectWrite生成
	m_directwrite = std::make_unique<DirectWrite>(&m_fontdata);
	m_directwrite->Init(Renderer::GetSwapChain());

	// リソースを読み込む
	resourceLoader();

	// キルストリークウィンドウ初期化
	//m_killWindow.SetWindowMs(2000);

	//Sound
	if (!m_audioInited)
	{
		m_audioInited = true;
		XAudSound::GetInstance()->soundInit();
		XAudSound::GetInstance()->soundBGMPlay((int)SoundBGAssets::Stage1);
	}

	// プレイヤ
	m_player = std::make_unique<player>(this);
	m_player->init();

	// 丸影
	m_blobshadow = std::make_unique<BlobShadow>();
	m_blobshadow->Init("assets/texture/effect000.jpg");
	m_blobshadow->ReserveBatch(ENEMYMAX);
	m_blobshadow->SetHeightOffset(0.5f);

	//m_aim = std::make_unique<aim>();
	//m_aim->init();

	// フィールド初期化
	m_field = std::make_unique<field>();
	m_field->init();

	// タワー初期化
	m_tower = std::make_unique<tower>(this);
	m_tower->init();

	for (int i = 0; i < ENEMYMAX; ++i)
	{
		m_enemies[i] = std::make_unique<enemy>(this);
		m_enemies[i]->init();

	}

	// 乱数エンジンを初期化
	auto& rng = RandomEngine::tls();
	rng.uniformReal(-500, 500);

	//Stage
	m_stage = Stage::Stage1;
	SetupStage(m_stage);

	// 弾プール（PLAYERBULLETMAX個）
	for (uint8_t i = 0; i < PLAYERBULLETMAX; ++i)
	{
		m_playerBullets[i] = std::make_unique<PlayerBullet>();
		m_playerBullets[i]->init();
		m_playerBullets[i]->SetActorNo(i);
	}
	//リソースプール
	for (uint8_t i = 0; i < RESOURCEMAX; ++i)
	{
		m_resource[i] = std::make_unique<Resource>();
		m_resource[i]->init();
		m_resource[i]->SetPlayer(getPlayer());
		m_resource[i]->SetField(getfield());
		m_resource[i]->Kill();
	}

	static const std::array<Vector2, 4> UV_FULL = {
	Vector2(0, 0), Vector2(1, 0),
	Vector2(0, 1), Vector2(1, 1)
	};

	m_circle = std::make_unique<CSprite>(
		256, 256,
		"assets/texture/Circle256_o.png",
		UV_FULL
	);

	m_effectExplosion = std::make_unique<EffectSystem>();
	m_effectExplosion->init();
	SphereDrawerInit();

	// Door座標の当たり判定壁
	{
		// Door.fbx の描画に合わせた基準位置
		constexpr Vector3 DOOR_POS = Vector3(0.0f, 0.0f, 880.0f);

		// 平面サイズ
		constexpr float WALL_H = 200.0f;
		float wallW = 500.0f; // 可変:幅

		SRT srt{};
		srt.scale = Vector3(1.0f, 1.0f, 1.0f);
		srt.rot = Vector3(0.0f, 0.0f, 0.0f);

		// Yは地面Heightに合わせる
		Vector3 pos = DOOR_POS;
		pos.y = m_field->GetHeight2(pos);

		srt.pos = pos;

		m_walls[0] = std::make_unique<wall>(this, srt, wallW, WALL_H);
		m_walls[0]->setSRT(srt); // gameobject側へ反映
		m_walls[0]->init();      // 平面生成

		{
			SRT srt2 = srt;
			srt2.pos.z = -srt.pos.z;   // Z-方向にも
			m_walls[1] = std::make_unique<wall>(this, srt2, wallW, WALL_H);
			m_walls[1]->setSRT(srt2);
			m_walls[1]->init();
		}
	}
}

/**
 * @brief シーンの終了処理
 */
void TutorialScene::dispose(){
	XAudSound::GetInstance()->soundDispose();
}

//弾書き出し用ラッパ
int TutorialScene::GetFreeBulletCount() const
{
	int freeCount = 0;
	for (const auto& pb : m_playerBullets)
	{
		if (pb && !pb->GetisAlive()) ++freeCount;
	}
	return freeCount;
}

int TutorialScene::GetAliveBulletCount() const
{
	int aliveCount = 0;
	for (const auto& pb : m_playerBullets)
	{
		if (pb && pb->GetisAlive()) ++aliveCount;
	}
	return aliveCount;
}

int TutorialScene::GetAliveTrapCount() const
{
	int aliveTrap = 0;
	for (const auto& pb : m_playerBullets)
	{
		if (!pb || !pb->GetisAlive()) continue;
		if (pb->IsTrap()) ++aliveTrap;
	}
	return aliveTrap;
}

//リソーススポーンばらけ
void TutorialScene::SpawnPickup(const Vector3& pos, int count)
{
	auto& rng = RandomEngine::tls();

	for (int i = 0; i < count; ++i)
	{
		Vector3 p = pos;
		p.x += static_cast<float>(rng.uniformReal(-20.0, 20.0));
		p.z += static_cast<float>(rng.uniformReal(-20.0, 20.0));
		SpawnResource(p);
	}
}

//リソースを読み込む
void TutorialScene::resourceLoader()
{
	// 光源計算なしシェーダー
	std::unique_ptr<CShader> shader1 = std::make_unique<CShader>();
	shader1->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");
	MeshManager::RegisterShader<CShader>("lightshader", std::move(shader1));

	std::unique_ptr<CShader> shader2 = std::make_unique<CShader>();
	shader2->Create("shader/unlitTextureVS.hlsl", "shader/unlitTexturePS.hlsl");
	MeshManager::RegisterShader<CShader>("unlightshader", std::move(shader2));

	std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
	smesh->Load("assets/model/chara/Nekoo.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender = std::make_unique<CStaticMeshRenderer>();
	smeshrender->Init(*smesh);
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Nekoo.fbx", std::move(smeshrender));
	MeshManager::RegisterMesh<CStaticMesh>("Nekoo.fbx", std::move(smesh));

	std::unique_ptr<CStaticMesh> smesh2 = std::make_unique<CStaticMesh>();
	smesh2->Load("assets/model/chara/256Mouse.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender2 = std::make_unique<CStaticMeshRenderer>();
	smeshrender2->Init(*smesh2);
	MeshManager::RegisterMesh<CStaticMesh>("256Mouse.fbx", std::move(smesh2));
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("256Mouse.fbx", std::move(smeshrender2));

	std::unique_ptr<CStaticMesh> smesh3 = std::make_unique<CStaticMesh>();
	smesh3->Load("assets/model/chara/tora_v2.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender3 = std::make_unique<CStaticMeshRenderer>();
	smeshrender3->Init(*smesh3);
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("tora_v2.fbx", std::move(smeshrender3));
	MeshManager::RegisterMesh<CStaticMesh>("tora_v2.fbx", std::move(smesh3));

	std::unique_ptr<CStaticMesh> smesh4 = std::make_unique<CStaticMesh>();
	smesh4->Load("assets/model/chara/Heart_Brilliant.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender4 = std::make_unique<CStaticMeshRenderer>();
	smeshrender4->Init(*smesh4);
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Heart_Brilliant.fbx", std::move(smeshrender4));
	MeshManager::RegisterMesh<CStaticMesh>("Heart_Brilliant.fbx", std::move(smesh4));

	std::unique_ptr<CStaticMesh> smesh5 = std::make_unique<CStaticMesh>();
	smesh5->Load("assets/model/chara/Ghost.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender5 = std::make_unique<CStaticMeshRenderer>();
	smeshrender5->Init(*smesh5);
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Ghost.fbx", std::move(smeshrender5));
	MeshManager::RegisterMesh<CStaticMesh>("Ghost.fbx", std::move(smesh5));

	std::unique_ptr<CStaticMesh> smesh6 = std::make_unique<CStaticMesh>();
	smesh6->Load("assets/model/chara/WallEntrance.fbx", "assets/model/chara");
	std::unique_ptr<CStaticMeshRenderer> smeshrender6 = std::make_unique<CStaticMeshRenderer>();
	smeshrender6->Init(*smesh6);
	MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("WallEntrance.fbx", std::move(smeshrender6));
	MeshManager::RegisterMesh<CStaticMesh>("WallEntrance.fbx", std::move(smesh6));

	//背景モデル
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/tower/Only Tower.obj", "assets/model/tower/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("Tower", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Tower", std::move(srenderer));

		//ドア
		std::unique_ptr<CStaticMesh> smesh5 = std::make_unique<CStaticMesh>();
		smesh5->Load("assets/model/chara/Door.fbx", "assets/model/chara");
		std::unique_ptr<CStaticMeshRenderer> smeshrender5 = std::make_unique<CStaticMeshRenderer>();
		smeshrender5->Init(*smesh5);
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("Door.fbx", std::move(smeshrender5));
		MeshManager::RegisterMesh<CStaticMesh>("Door.fbx", std::move(smesh5));
	}
}