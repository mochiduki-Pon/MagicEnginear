#include    "../system/CDirectInput.h"
#include	"../system/meshmanager.h"
#include	"../scene/TutorialScene.h"
#include "../utility/enumdebug.h"
#include	"../system/SphereDrawer.h"
#include	"../sound.h"
#include	"field.h"
#include	"player.h"

void player::init() {

	m_mesh = MeshManager::getMesh<CStaticMesh>("Nekoo.fbx");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("Nekoo.fbx");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
	//LIGHT light = Renderer::GetLight();
	//light.Direction = Vector4(0.0f, -0.3f, -1.0f, 0.0f); // 手前から当てるつもり
	//light.Direction.Normalize();
	//Renderer::SetLight(light);

	float rad = DirectX::XM_PI / 180.0f;
	m_srt.pos = Vector3(0, 0, 0);
	m_srt.scale = Vector3(0.25, 0.25, 0.25);
	m_srt.rot = Vector3(0, 180 * rad, 0);

	m_actstate = PlayerActState::Idle;

	// 被弾無敵クールダウン
	m_dmgCD.SetInterval(Time::Ms(DmgInvincible));
	m_dmgCD.Reset();

	// 弾種クールダウン
	for (size_t i = 0; i < m_bulletcd.size(); ++i)
{
    const auto no = static_cast<BulletGimmick::BulletNo>(i);
    const auto& sp = BulletGimmick::Spec(no);

	m_bulletcd[i].SetInterval(Time::Ms(sp.isTrap ? 1000 : 700));
    m_bulletcd[i].Reset();
	m_selectedNo = BulletGimmick::BulletNo::WaterShot;
	m_bulletMode = BulletMode::Shot;
}
}

// リソース回収
void player::AddMp(int amount)
{
	if (amount <= 0) return;

	m_mp += amount;
	m_mpDelta += amount;
	if (m_mp > m_maxMp)
		m_mp = m_maxMp;
}

//UI用
int player::ConsumeMpDelta()
{
	int d = m_mpDelta;
	m_mpDelta = 0;
	return d;
}



// 障害物とのヒットチェック
bool player::obstacleshitcheck() {

	bool hitflag = false;

	// 障害物との当たり判定
	SRT playersrt = this->getSRT();

	float playerwidth = this->getwidth();
	float playerheight = this->getheight();
	float playerdepth = this->getdepth();

	// OOBと球の当たり判定を行う(今障害物OBBのみ）
	Call::Collision::BoundingBoxOBB playerobb{};
	playerobb = Call::Collision::SetOBB(playersrt.rot, playersrt.pos, playerwidth, playerheight, playerdepth);

	// 障害物を取得
	std::vector<obstacle*> obstacles{};
	//obstacles = ((CameraBasicScene*)m_ownerscene)->getobstacles();
	//obstacles = ((TutorialScene*)m_ownerscene)->getobstacles();

	// すべてのOBBと球の距離を求める
	for (auto& obs : obstacles)
	{
		float width, height, depth;

		SRT srt = obs->getSRT();

		width = obs->getwidth();
		height = obs->getheight();
		depth = obs->getdepth();

		// OOBと球の当たり判定を行う(今障害物OBBのみ）
		Call::Collision::BoundingBoxOBB obsobb;
		obsobb = Call::Collision::SetOBB(srt.rot, srt.pos, width, height, depth);

		bool sts = Call::Collision::CollisionOBB(playerobb, obsobb);
		if (sts) {
			m_move = Vector3(0, 0, 0);
			hitflag = true;
			break;
		}
	}

	return hitflag;
}

// 壁とのヒットチェック
bool player::wallshitcheck(std::vector<wall::WallCollision>& hitwalls)
{
    bool hitflag = false;

    const Vector3 nextpos = m_srt.pos + m_move;
    const float radius = m_radius; // プレイヤ半径

    // 壁を取得
    std::vector<wall*> walls{};
    walls = ((TutorialScene*)m_ownerscene)->getwalls(); // 今使うScene

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

        // めり込み深い対策：壁法線方向へ逃がす
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

        // スライド量（壁法線方向成分を削除）
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

bool player::CanUseTrap(int cost) const
{
	return (cost > 0) && (m_mp >= cost);
}

bool player::ConsumeMp(int cost)
{
	if (!CanUseTrap(cost)) return false;
	m_mp -= cost;
	if (m_mp < 0) m_mp = 0; // 念のため
	return true;
}

void player::Knockback(const Vector3& fromPos)
{
	//m_onground = false;
	Vector3 dir = m_srt.pos - fromPos;
	dir.y = 0.0f;

	if (dir.LengthSquared() < 1e-6f)
		dir = Vector3(0, 0, 1);

	dir.Normalize();

	const float kbH = 10.0f;
	m_move = Vector3(0, 0, 0);
	m_move += dir * kbH;
}

Vector3 player::GetColliderCenter() const
{
	return m_srt.pos + Vector3(0, GetCollisionRadius(), 0);
}

bool player::Damage(int amount)
{
	const auto now = Time::Get().Now();

	const bool canDamage =
		(amount > 0) &&
		(m_hp > 0) &&
		m_dmgCD.Consume(now);

	if (!canDamage) return false;

	XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::SeDmg1);

	m_hp -= amount;
	if (m_hp < 0) m_hp = 0;
	return true;
}

//被弾処理
void player::OnDamage()
{
	if (m_playersts == PlayerState::Dead) return;

	m_react = PlayerReact::Damage;
	m_damageRemainMs = DAMAGE_FLASH_MS;
}

void player::PlayerHandle() {

	m_Acceleration = Vector3(0, 0, 0);				// 加速度０クリア

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W))
		{// 左前移動

			float radian;
			radian = PI * 0.75f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;

		}
		else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S))
		{// 左後移動

			float radian;
			radian = PI * 0.25f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;
		}
		else
		{// 左移動

			float radian;
			radian = PI * 0.50f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;
		}
	}

	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D))
	{
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
			// 右前移動

			float radian;
			radian = -PI * 0.75f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;

		}
		else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S))
		{// 右後移動
			float radian;
			radian = -PI * 0.25f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;
		}
		else
		{// 右移動

			float radian;
			radian = -PI * 0.50f;

			m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
			m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

			// 目標角度をセット
			m_destrot.y = radian;
		}
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W))
	{// 前移動
		float radian;
		radian = PI;

		m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
		m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

		// 目標角度をセット
		m_destrot.y = PI;
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S))
	{// 後移動
		float radian;
		radian = 0.0f;

		m_move.x -= sinf(radian) * VALUE_MOVE_MODEL;
		m_move.z -= cosf(radian) * VALUE_MOVE_MODEL;

		// 目標角度をセット
		m_destrot.y = 0.0f;
	}

	if (m_move.LengthSquared() > 0.0f) {
		m_actstate = PlayerActState::Walk;
	}

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RIGHT))
	{// 左回転
		m_destrot.y = m_srt.rot.y - VALUE_ROTATE_MODEL;
		if (m_destrot.y < -PI)
		{
			m_destrot.y += PI * 2.0f;
		}
	}

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_LEFT))
	{// 右回転
		m_destrot.y = m_srt.rot.y + VALUE_ROTATE_MODEL;
		if (m_destrot.y > PI)
		{
			m_destrot.y -= PI * 2.0f;
		}
	}

	// 目標角度と現在角度との差分を求める
	float diffrot = m_destrot.y - m_srt.rot.y;
	if (diffrot > PI)
	{
		diffrot -= PI * 2.0f;
	}
	if (diffrot < -PI)
	{
		diffrot += PI * 2.0f;
	}

	// 比率計算
	m_srt.rot.y += diffrot * RATE_ROTATE_MODEL;
	if (m_srt.rot.y > PI)
	{
		m_srt.rot.y -= PI * 2.0f;
	}
	if (m_srt.rot.y < -PI)
	{
		m_srt.rot.y += PI * 2.0f;
	}

	if (m_move.LengthSquared() < 0.0001f) {
		m_move = Vector3(0, 0, 0);
	}

}

void player::JumpHandle() {
	// space押下でジャンプ
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_SPACE) && m_onground)
	{
		m_Velocity.y = 12.0f;
		m_onground = false;
		m_jumpFlg = true;

		XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::Sejump);
		m_actstate = PlayerActState::Jump;
		std::cout << m_actstate << std::endl;

	}
}

void player::ChangeBullet()
{
	m_bulletMode = (m_bulletMode == BulletMode::Shot) ? BulletMode::Trap : BulletMode::Shot;
	m_selectedNo = (BulletGimmick::BulletNo)((uint8_t)m_element * 2u + (uint8_t)m_bulletMode);
}

BulletGimmick::BulletNo player::GetSelectedNo() const
{
	return m_selectedNo;
}

void player::ShotHandle()
{
	const bool trigger =
		CDirectInput::GetInstance().GetMouseLButtonTrigger() ||
		CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_B);

	m_shotFlg = false;

	auto& cd = m_bulletcd[ToIndex(m_bulletMode)];

	if (trigger && cd.Consume(Time::Get().Now()))
	{
		m_shotFlg = true;
		XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::shot1);
		std::cout << "[Shoot] selectedNo=" << (int)GetSelectedNo() << "\n";
	}
}

void player::CheckShotDir() {

	Vector3 dir;
	dir.x = -sinf(m_srt.rot.y);
	dir.y = 0.0f;
	dir.z = -cosf(m_srt.rot.y);

	dir.Normalize();

	SetPlayerDir(dir);   // m_vDir に反映
}

static BulletGimmick::BulletNo ErementMode(int elementIndex, player::BulletMode mode)
{
	// elementIndex: 0=Water, 1=Fire, 2=Wind
	const int base = elementIndex * 2;			// 0,2,4偶数番Shot
	const int m = (mode == player::BulletMode::Trap) ? 1 : 0;
	return (BulletGimmick::BulletNo)(base + m);
}

void player::HandleElement()
{
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_1))
	{
		//XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::change);
		m_element = BulletGimmick::Element::Water;
		m_selectedNo = (BulletGimmick::BulletNo)((uint8_t)m_element * 2u + (uint8_t)m_bulletMode);
	}

	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_2))
	{
		//XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::change);
		m_element = BulletGimmick::Element::Fire;
		m_selectedNo = (BulletGimmick::BulletNo)((uint8_t)m_element * 2u + (uint8_t)m_bulletMode);
	}

	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_3))
	{
		//XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::change);
		m_element = BulletGimmick::Element::Wind;
		m_selectedNo = (BulletGimmick::BulletNo)((uint8_t)m_element * 2u + (uint8_t)m_bulletMode);
	}
}

void player::update(uint64_t dt)
{
	//fieldとの当たり判定
	field* field = ((TutorialScene*)m_ownerscene)->getfield();
	m_spawnPos = m_srt.pos;

	// ジャンプ
	m_Acceleration = Vector3(0, 0, 0);// 加速度０クリア

	CheckShotDir();
	ShotHandle();
	HandleElement();

	switch (m_actstate)
	{
	case PlayerActState::Idle:

		PlayerHandle();
		JumpHandle();
		if (m_prevActState != PlayerActState::Idle) {
			std::cout << m_actstate << std::endl;
			m_prevActState = m_actstate;

		}

		break;

	case PlayerActState::Walk:
		PlayerHandle();
		JumpHandle();

		if (m_move.LengthSquared() < 0.0001f) {
			m_actstate = PlayerActState::Idle;
		}

		break;

	case PlayerActState::Jump:
		PlayerHandle();

		if (m_onground) { m_actstate = PlayerActState::Idle; }
		break;
	}

	if (m_prevActState != m_actstate) {
		std::cout << m_actstate << std::endl;
		m_prevActState = m_actstate;
	}

	switch (m_react)
	{
	case PlayerReact::None:
		
		break;

	case PlayerReact::Damage:
		timeutil::SubTime(m_damageRemainMs, dt);
		if (m_damageRemainMs == 0)
			m_react = PlayerReact::None;
		break;
	}

	//背景当たり判定部分
	bool obstaclehit = false;
	Vector3 nextpos = m_srt.pos + m_move; // 次の位置
	float radius = 10.0f;

	std::vector<wall::WallCollision> hitwalls{};

	// 壁との当たり判定
	bool hit = wallshitcheck(hitwalls);

    if (hit)
    {
        // 複数ヒットは合算（簡易）
        Vector3 push(0, 0, 0);
        Vector3 slide = m_move;

        for (const auto& h : hitwalls)
        {
            push += h.Penetration;

            // まずは最後の壁のスライド
            slide = h.Sliding;
        }

        m_srt.pos += push;
        m_move = slide;

        // スライド後に少し減衰
        m_move += -m_move * RATE_MOVE_MODEL;
    }
    else
    {
        m_srt.pos += m_move;

        if (field && field->FallingHole(m_srt.pos)) {
            m_onground = false;
            m_jumpFlg = false;
            m_groundheight = field->GetHoleHeight();
        }

        m_move += -m_move * RATE_MOVE_MODEL;
    }

	// Shot（実時間: モード別Cooldown + 自前トリガー）
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_Q))
	{
		XAudSound::GetInstance()->soundSEPlay((int)SoundSEAssets::change);
		ChangeBullet();
	}

	float h = field->GetHeight2(m_srt.pos);		// フィールドの高さを取得

	// 地面への吸いつき防止だがジャンプ中すり抜ける
	if (m_onground) {
		m_groundheight = h;
	}

	if (!m_onground) {
		m_Acceleration += m_gravity * 0.5f;					// 重力加速度を加える
	}
	const float maxFall = -2.0f;

	m_Velocity += m_Acceleration;

	m_srt.pos.x += m_Velocity.x;
	m_srt.pos.y += m_Velocity.y;
	m_srt.pos.z += m_Velocity.z;

	if (m_Velocity.y < maxFall) m_Velocity.y = maxFall;

	if (m_Velocity.y < 0.0f && m_srt.pos.y < m_groundheight) {
		m_Velocity.y = 0.0f;
		m_srt.pos.y = m_groundheight;
		m_onground = true;
		m_jumpFlg = false;
	}

	// リセット
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
	{// リセット
		m_srt.pos = Vector3(0.0f, 0.0f, 0.0f);
		m_srt.rot = Vector3(0.0f, 0.0f, 0.0f);
	}


	//	m_srt.pos = m_Pos;

	Matrix4x4 mtx = m_srt.GetMatrix();
}


void player::draw(uint64_t dt) {

	//// debug用当たり判定可視化
	//Vector3 center = GetColliderCenter();
	//SphereDrawerDraw(
	//	GetCollisionRadius(),
	//	Color(0, 0, 1, 0.4f),   // 青・半透明
	//	center.x,
	//	center.y,
	//	center.z

	//);

	Matrix4x4 mtx = m_srt.GetMatrix();
	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();

	// 被弾フラッシュ：残り時間の比率で明るさを減衰
	if (m_react == PlayerReact::Damage && m_damageRemainMs > 0)
	{
		const float remainRatio =
			static_cast<float>(m_damageRemainMs) / static_cast<float>(DAMAGE_FLASH_MS); // 1→0

		// 1.0～HIT_FLASH_MAX_MULTIPLIER の範囲で白く増光
		const float flash =
			1.0f + (HIT_FLASH_MAX_MULTIPLIER - 1.0f) * remainRatio;

		m_meshrenderer->Draw(Color(flash, 1.0f, 1.0f, 1.0f));
		return;
	}
	else
	{
		m_meshrenderer->Draw();
	}

}

void player::dispose() {
	XAudSound::GetInstance()->soundDispose();
}

bool player::ConsumeShotFlag()
{
	bool f = m_shotFlg;
	m_shotFlg = false;
	return f;
}