#include    "../system/CDirectInput.h"
#include	"../system/meshmanager.h"
#include	"../system/SphereDrawer.h"
#include	"../system/motiondatamanager.h"
#include	"../scene/TutorialScene.h"
#include	"enemy.h"
#include	"tower.h"
#include	"../sound.h"
#include	"../utility/MathUtil.h"
#include	"../system/EnemyCounter.h"
#include	"../gameobject/resource.h"

//控え選手状態
void enemy::init()
{
	//描画リソース
	//m_mesh = MeshManager::getMesh<CStaticMesh>("Ghost.fbx");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("256Mouse.fbx");
	m_shader = MeshManager::getShader<CShader>("lightshader");

	m_srt.scale = Vector3(0.7f, 0.7f, 0.7f);

	//ID登録
	m_id = s_nextId++;
	m_sts = Status::Dead;
	m_dead = true;
	m_spawned = false;
}

//スポーン処理
void enemy::Spawn(const Vector3& pos)
{
	// WaveActive
	if (!EnemyCounter::GetInstance().IsWaveActive())
		return;

	m_spawned = true; // 出場決定
	m_sts = Status::Alive;
	m_dead = false;
	m_hp = m_maxHp;
	m_deadRemainMs = 0;

	m_reaction = Reaction::None;
	m_damageRemainMs = 0;
	m_damageImpulseDone = false;
	m_Velocity = Vector3(0, 0, 0);
	m_Acceleration = Vector3(0, 0, 0);
	m_onground = true;
	m_boostFrames = 0;

	m_srt.rot.x = 0.0f;
	m_srt.rot.z = 0.0f;

	SRT srt = getSRT();
	srt.pos = pos;
	setSRT(srt);

	std::cout << "[SPAWN] id=" << m_id
		<< " rot=("
		<< m_srt.rot.x << ", "
		<< m_srt.rot.y << ", "
		<< m_srt.rot.z << ")"
		<< std::endl;

	EnemyCounter::GetInstance().RegisterEnemy(m_id);
}

void enemy::SetSpawned(bool v)
{
	m_spawned = v;

	if (!v)
	{
		m_sts = Status::Dead;
		m_reaction = Reaction::None;
		m_move = Vector3(0, 0, 0);
	}
}

Vector3 enemy::GetColliderCenter() const
{
	return m_srt.pos + Vector3(0, GetCollisionRadius(), 0);
}

void enemy::SeparateFromOthers()
{
	auto* scn = static_cast<TutorialScene*>(m_ownerscene);
	if (!scn) return;

	const auto& enemies = scn->GetEnemies();

	constexpr float SEP_R = 40.0f;		// 分離半径
	constexpr float MAX_PUSH = 1.5f;	// 1フレーム最大押し戻し
	constexpr float K = 0.15f;			// 押しの強さ

	Vector3 push(0, 0, 0);

	for (const auto& e : enemies)
	{
		if (!e) continue;
		if (e.get() == this) continue;
		if (!e->IsAlive()) continue;

		Vector3 d = m_srt.pos - e->getSRT().pos;
		d.y = 0.0f;					// 水平だけで押す

		const float d2 = d.LengthSquared();
		if (d2 < 1e-6f) continue;	// ほぼ同一点は方向が取れないのでスキップ

		const float dist = std::sqrt(d2);
		if (dist >= SEP_R) continue; // 遠いなら関係なし

		// 押し方向（相手→自分）
		const Vector3 n = d * (1.0f / dist);

		// 重なり量（近いほど大きい）
		const float overlap = (SEP_R - dist);

		// 重なりに比例して押す
		push += n * (overlap * K);
	}

	// push の長さを MAX_PUSH にクランプ（向きは維持）
	const float len2 = push.LengthSquared();
	const float max2 = MAX_PUSH * MAX_PUSH;
	if (len2 > max2)
	{
		push *= (MAX_PUSH / std::sqrt(len2));
	}

	m_srt.pos += push;
}

// 壁とのヒットチェック
bool enemy::wallshitcheck(std::vector<wall::WallCollision>& hitwalls)
{
    bool hitflag = false;

    const Vector3 nextpos = m_srt.pos + m_move;
    const float radius = 10.0f; // プレイヤ半径

    // 壁を取得
    std::vector<wall*> walls{};
    walls = ((TutorialScene*)m_ownerscene)->getwalls(); // 使うScene

    for (auto* w : walls)
    {
        if (!w) continue;

        w->clearhitflag();

        // 壁OBB
        const SRT wsrt = w->getSRT();
        const float ww = w->getwidth();
        const float wh = w->getheight();
        constexpr float WALL_THICK = 2.0f;

        Call::Collision::BoundingBoxOBB wallObb =
            Call::Collision::SetOBB(wsrt.rot, wsrt.pos, ww, wh, WALL_THICK);

        // プレイヤ球
        Call::Collision::BoundingSphere sphere(nextpos, radius);

        // 球 vs OBB
        if (!Call::Collision::CollisionSphereOBB(sphere, wallObb))
            continue;

        // 最近接点
        wall::WallCollision wc{};
        wc.wall = w;
        wc.Penetration = Vector3(0, 0, 0);
        wc.Sliding = Vector3(0, 0, 0);
        Call::Collision::ClosestPtPointOBB(sphere.center, wallObb, wc.IntersectionPoint);

        // 押し戻しベクトル
        Vector3 v = sphere.center - wc.IntersectionPoint;
        const float d2 = v.LengthSquared();

        if (d2 < 1e-6f)
        {
            const PLANEINFO pi = w->getplaneinfo();
            Vector3 n(pi.plane.a, pi.plane.b, pi.plane.c);
            if (n.LengthSquared() < 1e-6f) n = Vector3(0, 0, 1);
            n.Normalize();
            wc.Penetration = n * radius;
        }
        else
        {
            const float d = std::sqrt(d2);
            v *= (1.0f / d);                 // n
            const float pen = (radius - d);  // 侵入量
            wc.Penetration = v * pen;
        }

        // スライド量
        Vector3 n = wc.Penetration;
        if (n.LengthSquared() > 1e-6f) n.Normalize();

        // move の法線方向成分を取り除く = 壁面に沿う
        wc.Sliding = m_move - n * m_move.Dot(n);

        w->sethitflag();
        hitwalls.push_back(wc);
        hitflag = true;
    }

    return hitflag;
}

// 止める！調整用
void enemy::ApplyWallAndMove()
{
    std::vector<wall::WallCollision> hitwalls;
    const bool hit = wallshitcheck(hitwalls);

    if (!hit)
    {
        m_srt.pos += m_move;
        m_move += -m_move * RATE_MOVE_MODEL;
        return;
    }

    // 押し戻し
    Vector3 push(0,0,0);
    for (const auto& h : hitwalls) push += h.Penetration;
    m_srt.pos += push;

    // ビターン
    Vector3 n = push;
    n.y = 0.0f;                      // 地上運用Y無視
    if (n.LengthSquared() < 1e-6f) n = Vector3(0,0,1);
    n.Normalize();

    const float e = 0.75f;           // 反発係数（調整ポイント）
    float vn = m_move.Dot(n);
    if (vn < 0.0f)                   // 壁に向かってる時だけ反射
    {
        m_move = m_move - (1.0f + e) * vn * n;
    }

    // 反射後減衰率
    m_move += -m_move * (RATE_MOVE_MODEL * 0.2f);
}

//外部引き渡しラップ（被弾入口）
void enemy::Damage(int amount)
{
	if (m_sts == Status::Dead) return;
	if (amount <= 0) return;

	std::cout << "[DMG] this=" << this
		<< " amount=" << amount
		<< " hp(before)=" << m_hp
		<< std::endl;

	OnDamage();
	m_hp -= amount;

	if (m_hp <= 0)
	{
		m_pendingKill = true;
	}

}

//被弾処理
void enemy::OnDamage()
{
	if (m_sts == Status::Dead) return;

	m_reaction = Reaction::Damage;
	m_damageRemainMs = DAMAGE_FLASH_MS;

	m_damageImpulseDone = false;

	m_boostFrames = 500;
}

//kill drop数Rand
int enemy::BaseDropRand()
{
	auto& rng = RandomEngine::tls();
	double r = rng.uniformReal(0.0, 1.0);
	if (r < 0.25) return 3;
	if (r < 0.28) return 4;
	return 2;
}

void enemy::Kill()
{
	if (m_sts == Status::Dead) return;
	m_sts = Status::Dead;

	if (m_dead) return;
	m_dead = true;

	if (m_deathCause == DeathCause::Normal || m_deathCause == DeathCause::Explosion)
	{
		int drop = BaseDropRand();

		if (m_deathCause == DeathCause::Explosion)
			drop = int(drop * m_explosionDropMul);

		if (m_ownerscene) m_ownerscene->SpawnPickup(m_srt.pos, drop);
	}

	// 次の死に持ち越さないように
	m_explosionDropMul = 1.0f;

	EnemyCounter::GetInstance().UnregisterEnemy(m_id);
	m_deadRemainMs = DEAD_VISIBLE_MS;
	m_onground = false;
	EnterDeath();
}

//死亡処理
void enemy::EnterDeath()
{
	std::cout << "[DEATH ENTER] id=" << m_id
		<< " rot(before)=("
		<< m_srt.rot.x << ", "
		<< m_srt.rot.y << ", "
		<< m_srt.rot.z << ")"
		<< std::endl;

	m_move = Vector3(0, 0, 0);

	// まずZ-方向に倒す
	m_srt.rot.x = -PI * 0.5f;

	// 埋まり補正
	const float lift = 10.0f * m_srt.scale.y;
	m_srt.pos.y += lift;

	// 死体を少し残す
	m_deadRemainMs = DEAD_VISIBLE_MS;
}

void enemy::Alive()
{
	// ブースト中ならターゲット位置取得
	if (m_boostFrames > 0)
	{
		player* pl = ((TutorialScene*)m_ownerscene)->getPlayer();
		if (pl) m_targetPos = pl->getSRT().pos;
		else    m_targetPos = ((TutorialScene*)m_ownerscene)->gettower()->getSRT().pos;
	}

	// 通常はタワー
	else
	{
		m_targetPos = ((TutorialScene*)m_ownerscene)->gettower()->getSRT().pos;
	}

	if (m_onground) Ground();
	else            Air();

	SeparateFromOthers();
	ApplyWallAndMove();
}

void enemy::Ground()
{
	Vector3 dir = m_targetPos - m_srt.pos;
	dir.y = 0.0f;

	if (dir.LengthSquared() < 1e-6f)
	{
		m_move = Vector3(0, 0, 0);
		return;
	}

	dir.Normalize();

	// 移動方向はターゲット
	m_move = dir * m_speed;
	m_srt.pos += m_move;

	// 向きも合わせる
	m_srt.rot.y = atan2f(-dir.x, -dir.z);

	m_Velocity.y = 0.0f;
}

void enemy::Air()
{
	const float maxFall = -2.0f;

	// 重力
	m_Acceleration = Vector3(0, 0, 0);
	m_Acceleration += m_gravity * 0.5f;

	m_Velocity += m_Acceleration;

	// 空中は velocity のみ
	m_srt.pos += m_Velocity;

	if (m_Velocity.y < maxFall) m_Velocity.y = maxFall;

	// 着地判定
	if (m_Velocity.y < 0.0f && m_srt.pos.y < m_groundheight) {
		m_Velocity.y = 0.0f;
		m_srt.pos.y = m_groundheight;
		m_onground = true;
	}
}

void enemy::HitBullet(const BulletGimmick::BulletSpec& sp, const Vector3& fromPos)
{
	m_hitFromExplosion = false;
	m_kbH = sp.kbH;
	m_kbV = sp.kbV;
	//m_hitFromPos = fromPos; // dir計算に使うなら
	m_damageImpulseDone = false;
	m_reaction = Reaction::Damage;
	m_damageRemainMs = DAMAGE_REMAIN_MS; // 既存の値
}

void enemy::update(uint64_t dt)
{
	field* field = ((TutorialScene*)m_ownerscene)->getfield();
	float h = field->GetHeight2(m_srt.pos);

	// 落とし穴判
	if (m_sts != Status::Dead && m_srt.pos.y < ENEMY_FALL_Y)
	{
		m_deathCause = DeathCause::Fall;
		Kill();     // Unregister
		return;
	}

	if (field && field->FallingHole(m_srt.pos))
	{
		m_onground = false;
		m_groundheight = field->GetHoleHeight(); // -200
	}
	else
	{
		float h = field->GetHeight2(m_srt.pos);
		if (m_onground) m_groundheight = h; // 接地中だけ更新
	}

	if (m_onground) m_srt.pos.y = m_groundheight;

	switch (m_sts)
	{
	case Status::Alive:
	{
		// 反応先に
		switch (m_reaction)
		{
		case Reaction::None:
			break;
		case Reaction::Damage:
		{
			if (!m_damageImpulseDone)
			{
				m_damageImpulseDone = true;
				
				// 方向計算
				 player* p = ((TutorialScene*)m_ownerscene)->getPlayer();
				Vector3 dir = m_srt.pos - p->getSRT().pos;
				dir.y = 0.0f;
				
				if (dir.LengthSquared() < 1e-6f)
				{
					dir = Vector3(0, 0, 1);
				}
				
				dir.Normalize();
				
				const auto& sp = BulletGimmick::Spec(m_lastHitBulletNo);
				// ノックバック
				m_kbH = sp.kbH;
				m_kbV = sp.kbV;
				// 爆発ノックバック
				if (m_hitFromExplosion)
				{
					m_kbH = 14.0f;
					m_kbV = 12.0f;
				}
				
				m_hitFromExplosion = false;
				
				// 速度に反映
				m_Velocity += dir * m_kbH;
				m_Velocity.y += m_kbV;
				m_onground = false;
			}

			if (m_damageRemainMs > 0)
			{
				timeutil::SubTime(m_damageRemainMs, dt);
			}

			if (m_damageRemainMs == 0)
			{
				m_reaction = Reaction::None;

				if (m_pendingKill)
				{
					m_pendingKill = false;
					Kill();
				}
			}
			break;
}

		case Reaction::Stun:
			break;

		case Reaction::Knockback:
			break;

		default:
			break;
		}

		// boost
		if (m_boostFrames > 0)
		{
			--m_boostFrames;

			const float orig = m_speed;
			m_speed = orig * 2.0f;
			Alive();
			m_speed = orig;
		}
		else
		{
			Alive();
		}

		break;
	}

	case Status::Dead:
	{
		// 重力
		m_Velocity.y += m_gravity.y * 0.5f;

		// XZ
		Vector3 moveXZ(m_Velocity.x, 0.0f, m_Velocity.z);

		std::vector<wall::WallCollision> hitwalls;
		const bool hit = wallshitcheck(hitwalls);

		if (hit)
		{
			Vector3 pushXZ(0, 0, 0);
			Vector3 slideXZ = moveXZ;

			for (const auto& h : hitwalls)
			{
				Vector3 p = h.Penetration; p.y = 0.0f;  // Y捨てる
				pushXZ += p;

				Vector3 s = h.Sliding; s.y = 0.0f;      // Y捨てる
				slideXZ = s;
			}

			m_srt.pos += pushXZ;
			moveXZ = slideXZ;
		}
		else
		{
			m_srt.pos += moveXZ;
		}

		// 速度へ反映（XZ）
		m_Velocity.x = moveXZ.x;
		m_Velocity.z = moveXZ.z;

		// Y
		m_srt.pos.y += m_Velocity.y;

		// 着地判定
		if (m_srt.pos.y <= m_groundheight)
		{
			m_srt.pos.y = m_groundheight;

			if (m_Velocity.y < 0.0f)
				m_Velocity.y = -m_Velocity.y * 0.25f;

			if (!m_landedOnce) {
				m_landedOnce = true;
				m_srt.scale.y *= 0.92f;
			}

			// 横は止める
			m_Velocity.x = 0.0f;
			m_Velocity.z = 0.0f;
		}

		if (m_deadRemainMs > 0) timeutil::SubTime(m_deadRemainMs, dt);
		break;
	}
	}
}

void enemy::draw(uint64_t dt)
{
	if (!m_spawned)
		return;

	//(void)dt;
	// debug用当たり判定可視化
	//Vector3 center = GetColliderCenter();
	//SphereDrawerDraw(
	//	GetCollisionRadius(),
	//	Color(1, 0, 0, 0.4f),   // 青・半透明
	//	center.x,
	//	center.y,
	//	center.z
	//);

	Matrix4x4 mtx = m_srt.GetMatrix();
	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();

	// 被弾フラッシュ：残り時間の比率で明るさを減衰
	if (m_reaction == Reaction::Damage && m_damageRemainMs > 0)
	{
		const float remainRatio =
			static_cast<float>(m_damageRemainMs) / static_cast<float>(DAMAGE_FLASH_MS); // 1→0

		// 1.0～HIT_FLASH_MAX_MULTIPLIER の範囲で白く増光
		const float flash =
			1.0f + (HIT_FLASH_MAX_MULTIPLIER - 1.0f) * remainRatio;

		m_meshrenderer->Draw(Color(flash, flash, flash, 1.0f));
		return;
	}
	else
	{
		m_meshrenderer->Draw();
		/*Color DiffuseColor = Color(1.0f, 0.0f, 0.0f, 0.4f);
		m_staticmeshrenderer->Draw(DiffuseColor);*/
	}
}

void enemy::dispose()
{
}