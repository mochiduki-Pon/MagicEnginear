#include	"../system/commontypes.h"
#include	"../system/meshmanager.h"
#include	"../system/SphereDrawer.h"
#include	"../utility/MathUtil.h"
#include	"../gameobject/player.h"
#include	<array>
#include	<iostream>
#include	"resource.h"

void Resource::init() {

	m_vertices.clear();

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
	vtx.Diffuse = Color(1, 1, 1, 0.6);
	vtx.Normal = Vector3(0, 0, -1);
	vtx.TexCoord = Vector2(u[1], v[1]);
	m_vertices.push_back(vtx);

	vtx.Position.x = -m_width / 2.0f;
	vtx.Position.y = 0.0f;
	vtx.Position.z = 0.0f;
	vtx.Diffuse = Color(0, 1, 1, 0.6);
	vtx.Normal = Vector3(0, 0, -1);
	vtx.TexCoord = Vector2(u[2], v[2]);
	m_vertices.push_back(vtx);

	vtx.Position.x = m_width / 2.0f;
	vtx.Position.y = 0.0f;
	vtx.Position.z = 0.0f;
	vtx.Diffuse = Color(0, 0, 1, 0.6);
	vtx.Normal = Vector3(0, 0, -1);
	vtx.TexCoord = Vector2(u[3], v[3]);
	m_vertices.push_back(vtx);

	// 頂点バッファ生成
	m_VertexBuffer.Create(m_vertices);

	// シェーダオブジェクト生成
	m_shader = MeshManager::getShader<CShader>("lightshader");

	// マテリアル生成
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = TRUE;

	m_Material.Create(mtrl);

	// テクスチャロード
	bool sts = m_Texture.Load("assets/texture/orblight_D.png");
	assert(sts == true);

	SRT srt = getSRT();
	m_srt.scale = { 1.5f,1.5f,1.5f };
	m_srt.pos = { 0,0,0 };
	m_srt.rot = { 0,0,0 };
	m_alive = false;
}

void Resource::Spawn(const Vector3& pos,int value, bool enableAttract)
{
	m_srt.pos = pos;
	m_alive = true;
	m_value = value;
	m_lifeTime = 0.0f;
	m_attract = enableAttract;

	m_spawnTime = Time::Get().Now();
}

//プレイヤーに集まる
void Resource::Attract(const Vector3& playerPos, uint64_t dtMs)
{
	const float dt = dtMs * 0.001f;

	Vector3 toP = playerPos - m_srt.pos;

	// XZ開始判定
	Vector3 toPXZ = toP;
	toPXZ.y = 0.0f;

	const float dist2 = toPXZ.LengthSquared();
	if (dist2 > START_R * START_R || dist2 < 1e-8f) return;

	// XZ
	toPXZ *= (1.0f / std::sqrt(dist2));   // normalize
	m_srt.pos += toPXZ * (SPEED * dt);

	// Y 弱め
	constexpr float Y_SPEED = 60.0f;		// 調整用 SPEEDより小さく持つ
	const float dyMax = Y_SPEED * dt;

	float dy = toP.y;
	if (dy > dyMax) dy = dyMax;
	if (dy < -dyMax) dy = -dyMax;

	m_srt.pos.y += dy;
}

void Resource::AttractToPlayer(uint64_t dtMs)
{
	if (!m_attract) return;
	if (!m_player) return;

	Attract(m_player->getSRT().pos, dtMs);
}

void Resource::update(uint64_t dt)
{
	if (!m_alive) return;

	// 賞味期限
	if (Time::Get().Now() - m_spawnTime > RESOURCE_LIFE_TIME)
	{
		m_alive = false;
		return;
	}

	// 奈落回収
	if (m_srt.pos.y < FALL_Y)
	{
		m_alive = false;
		return;
	}

	AttractToPlayer(dt);
}

void Resource::draw(uint64_t dt) {

	if (m_alive) {

		Matrix4x4 mtx = getSRT().GetMatrix();
		Renderer::SetWorldMatrix(&mtx);

		ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		m_shader->SetGPU();
		m_VertexBuffer.SetGPU();

		m_Material.SetGPU();
		m_Texture.SetGPU();

		devicecontext->Draw(4, 0);
	}
}

void Resource::drawbillboard(Camera* camera) {

	Matrix4x4 viewmtx = camera->GetViewMatrix();

	// 転置行列作成（viewの回転成分を取り出す目的）
	Matrix4x4 trasnposemtx = viewmtx.Transpose();

	Matrix4x4 billboardmtx = Matrix4x4::Identity;

	// ここがポイント：回転成分(3x3)にスケールを掛ける
	const float sx = m_srt.scale.x;
	const float sy = m_srt.scale.y;
	const float sz = m_srt.scale.z;

	billboardmtx._11 = trasnposemtx._11 * sx;
	billboardmtx._12 = trasnposemtx._12 * sx;
	billboardmtx._13 = trasnposemtx._13 * sx;

	billboardmtx._21 = trasnposemtx._21 * sy;
	billboardmtx._22 = trasnposemtx._22 * sy;
	billboardmtx._23 = trasnposemtx._23 * sy;

	billboardmtx._31 = trasnposemtx._31 * sz;
	billboardmtx._32 = trasnposemtx._32 * sz;
	billboardmtx._33 = trasnposemtx._33 * sz;

	billboardmtx._41 = m_srt.pos.x;
	billboardmtx._42 = m_srt.pos.y;
	billboardmtx._43 = m_srt.pos.z;

	Renderer::SetWorldMatrix(&billboardmtx);

	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_shader->SetGPU();
	m_VertexBuffer.SetGPU();

	m_Material.SetGPU();
	m_Texture.SetGPU();

	devicecontext->Draw(4, 0);
}

void Resource::dispose() {
}