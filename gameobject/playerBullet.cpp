#include	"playerBullet.h"	
#include	"../system/commontypes.h"
#include	"../system/meshmanager.h"
#include	<array>
#include	<iostream>
#include	"../system/SphereDrawer.h"

void PlayerBullet::init() {

	float u[4] = { 0,    //左上
					1.0f, //右上
					0,    //左下
					1.0f };//右下
	float v[4] = { 0,
					0,
					1.0f,
					1.0f };

	
		VERTEX_3D vtx{};

		vtx.Position.x = -m_width / 2.0f;
		vtx.Position.y = m_height;
		vtx.Position.z = 0.0f;
		vtx.Diffuse = Color(1.0f, 1.0f, 1.0f, 0.6f);
		vtx.Normal = Vector3(0, 0, -1);
		vtx.TexCoord = Vector2(u[0], v[0]);
		m_vertices.push_back(vtx);

		vtx.Position.x = m_width / 2.0f;
		vtx.Position.y = m_height;
		vtx.Position.z = 0.0f;
		vtx.Diffuse = Color(1.0f, 1.0f, 1.0f, 0.6f);
		vtx.Normal = Vector3(0, 0, -1);
		vtx.TexCoord = Vector2(u[1], v[1]);
		m_vertices.push_back(vtx);

		vtx.Position.x = -m_width / 2.0f;
		vtx.Position.y = 0.0f;
		vtx.Position.z = 0.0f;
		vtx.Diffuse = Color(0.0f, 1.0f, 1.0f, 0.6f);
		vtx.Normal = Vector3(0, 0, -1);
		vtx.TexCoord = Vector2(u[2], v[2]);
		m_vertices.push_back(vtx);

		vtx.Position.x = m_width / 2.0f;
		vtx.Position.y = 0.0f;
		vtx.Position.z = 0.0f;
		vtx.Diffuse = Color(0.0f, 0.0f, 1.0f, 0.6f);
		vtx.Normal = Vector3(0, 0, -1);
		vtx.TexCoord = Vector2(u[3], v[3]);
		m_vertices.push_back(vtx);

	

	// 頂点バッファ生成
	m_VertexBuffer.Create(m_vertices);

	// シェーダオブジェクト生成
	m_shader = MeshManager::getShader<CShader>("lightshader");

	// マテリアル生成
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0.15f, 0.15f, 0.05f, 0);
	//mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1.2f, 1.2f, 1.2f, 1.0f);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = TRUE;

	m_Material.Create(mtrl);
	// テクスチャロード
	bool sts = m_Texture.Load("assets/texture/Circle256.png");
	assert(sts == true);
	m_currentTexturePath = "assets/texture/Circle256.png";
	

	SRT srt=getSRT();
	srt.scale = { 2,2,2 };
	srt.pos = { 0,0,0 };
	srt.rot = { 0,0,0 };

	setSRT(srt);
	isAlive = false;
	m_triggered = false;

}

// プーリング対策
void PlayerBullet::ResetForSpawn()
{
	m_tick = 0.0f;
	isAlive = true;
	m_triggered = false;
	m_spawnAt = Time::Get().Now();                          // 開始時刻
	m_maxLifeMs = static_cast<uint64_t>(m_maxLife * 1000.0f);// 寿命(ms)
}

// ビジュアル用バーテクス
void PlayerBullet::VisualGimmick()
{
	const auto& vs = BulletGimmick::VSpec(m_no);

	if (m_vertices.size() >= 4)
	{
		m_vertices[0].Diffuse = vs.c0;
		m_vertices[1].Diffuse = vs.c1;
		m_vertices[2].Diffuse = vs.c2;
		m_vertices[3].Diffuse = vs.c3;

		m_VertexBuffer.Create(m_vertices);
	}

	// テクスチャ変更
	if (vs.texture != nullptr && vs.texture[0] != '\0' && vs.texture != m_currentTexturePath)
	{
		const bool sts = m_Texture.Load(vs.texture);
		assert(sts == true);
		m_currentTexturePath = vs.texture;
	}
}

Vector3 PlayerBullet::GetColliderCenter() const
{
	return m_srt.pos + Vector3(0, GetCollisionRadius(), 0);
}

// 爆発センター
Vector3 PlayerBullet::GetExplosionCenter() const
{
	return m_srt.pos; // 足元起爆修正
}

// 爆発半径
float PlayerBullet::GetExplosionRadius() const
{
	return BulletGimmick::Spec(m_no).radius;
}

// 爆発したか？
void PlayerBullet::Explode()
{
	if (m_triggered) return;
	m_triggered = true;

	std::cout << "TRAP EXPLODE\n";
	Kill();
}

void PlayerBullet::Spawn(const Vector3& pos, const Vector3& dir, BulletGimmick::BulletNo no)
{
	// 発射位置
	SRT srt = getSRT();
	srt.pos = pos;
	setSRT(srt);

	SetDir(dir);

	m_no = no;
	m_spec = BulletGimmick::Spec(m_no);

	m_speed = m_spec.speed;
	m_maxLife = m_spec.lifeSec;
	m_radius = m_spec.radius;

	// 見た目
	VisualGimmick();
	ResetForSpawn();

	std::cout << "[PB Spawn] no=" << (int)m_no
		<< " dmg=" << BulletGimmick::Spec(m_no).damage
		<< " vScale=" << BulletGimmick::VSpec(m_no).scale
		<< "\n";
	std::cout << "BulletNo changed to: " << (int)m_no << std::endl;
}

void PlayerBullet::update(uint64_t dt)
{
	if (!isAlive) return;

	// 寿命チェック
	const uint64_t elapsed = Time::ElapsedMs(m_spawnAt);
	if (elapsed >= m_maxLifeMs)
	{
		isAlive = false;
		return;
	}

	//Trap部分
	if (m_spec.isTrap)
	{
		// Trap 挙動
		return;
	}

	// shot挙動
	SRT srt = getSRT();

	if (m_no == BulletGimmick::BulletNo::FireShot)
	{
		srt.pos += m_dir * m_speed;
	}
	else
	{
		// デフォシャボン玉
		Vector3 up(0, 1, 0);
		m_dir = Lerp(m_dir, up, 0.005f);
		m_dir.Normalize();
		srt.pos += m_dir * m_speed;
	}

	setSRT(srt);
}

void PlayerBullet::draw(uint64_t dt)
{
	if (!isAlive) return;

	// 見た目Spec
	const auto& vs = BulletGimmick::VSpec(m_no);

	// 経過秒
	const float ageSec = Time::ElapsedMs(m_spawnAt) * 0.001f;

	SRT drawSrt = getSRT();
	float s = vs.scale;

	// 点滅
	if (vs.pulse)
	{
		const float t = sinf(ageSec * vs.pulseSpeed) * 0.5f + 0.5f;
		s *= (1.0f + vs.pulseAmp * t);
	}

	drawSrt.scale *= Vector3(s, s, s);
	Matrix4x4 mtx = drawSrt.GetMatrix();
	Renderer::SetWorldMatrix(&mtx);

	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_shader->SetGPU();
	m_VertexBuffer.SetGPU();
	m_Material.SetGPU();
	m_Texture.SetGPU();

	devicecontext->Draw(4, 0);

	// debug用当たり判定可視化
	Vector3 center = GetColliderCenter();
	SphereDrawerDraw(GetCollisionRadius(), Color(1, 1, 1, 0.3f),
		center.x, center.y, center.z);
}

void PlayerBullet::drawbillboard(Camera* camera) {
	
	Matrix4x4 viewmtx = camera->GetViewMatrix();

	// 転置行列作成
	Matrix4x4 trasnposemtx = viewmtx.Transpose();

	Matrix4x4 billboardmtx = Matrix4x4::Identity;
	billboardmtx._11 = trasnposemtx._11;
	billboardmtx._12 = trasnposemtx._12;
	billboardmtx._13 = trasnposemtx._13;

	billboardmtx._21 = trasnposemtx._21;
	billboardmtx._22 = trasnposemtx._22;
	billboardmtx._23 = trasnposemtx._23;

	billboardmtx._31 = trasnposemtx._31;
	billboardmtx._32 = trasnposemtx._32;
	billboardmtx._33 = trasnposemtx._33;

	billboardmtx._41 = m_srt.pos.x;
	billboardmtx._42 = m_srt.pos.y;
	billboardmtx._43 = m_srt.pos.z;

	Renderer::SetWorldMatrix(&billboardmtx);

	// 描画の処理
	ID3D11DeviceContext* devicecontext;
	devicecontext = Renderer::GetDeviceContext();

	// トポロジーをセット（旧プリミティブタイプ）
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_shader->SetGPU();
	m_VertexBuffer.SetGPU();

	m_Material.SetGPU();
	m_Texture.SetGPU();

	devicecontext->Draw(
		4,
		0);

}

void PlayerBullet::dispose() {

}

void PlayerBullet::SetSpeed(float s) { m_speed = s; }

float PlayerBullet::GetLifeTime() const {
	return m_lifeTime;
}

float PlayerBullet::GetMaxLife() const {
	return m_maxLife;
}
