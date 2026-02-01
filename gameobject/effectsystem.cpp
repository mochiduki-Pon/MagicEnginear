#include	"effectsystem.h"	
#include	"../system/commontypes.h"
#include	"../system/meshmanager.h"
#include	<array>
#include	<iostream>
#include	"player.h"

//void EffectSystem::init() {
//
//	float u[4] = { 0,    //左上
//					1.0f, //右上
//					0,    //左下
//					1.0f };//右下
//	float v[4] = { 0,
//					0,
//					1.0f,
//					1.0f };
//
//	VERTEX_3D vtx{};
//
//	vtx.Position.x = -m_width / 2.0f;
//	vtx.Position.y = m_height;
//	vtx.Position.z = 0.0f;
//	vtx.Diffuse = Color(1, 1, 1, 0.6);
//	vtx.Normal = Vector3(0, 0, -1);
//	vtx.TexCoord = Vector2(u[0], v[0]);
//	m_vertices.push_back(vtx);
//
//	vtx.Position.x = m_width / 2.0f;
//	vtx.Position.y = m_height;
//	vtx.Position.z = 0.0f;
//	vtx.Diffuse = Color(1, 1, 1, 0.6);
//	vtx.Normal = Vector3(0, 0, -1);
//	vtx.TexCoord = Vector2(u[1], v[1]);
//	m_vertices.push_back(vtx);
//
//	vtx.Position.x = -m_width / 2.0f;
//	vtx.Position.y = 0.0f;
//	vtx.Position.z = 0.0f;
//	vtx.Diffuse = Color(0, 1, 1, 0.6);
//	vtx.Normal = Vector3(0, 0, -1);
//	vtx.TexCoord = Vector2(u[2], v[2]);
//	m_vertices.push_back(vtx);
//
//	vtx.Position.x = m_width / 2.0f;
//	vtx.Position.y = 0.0f;
//	vtx.Position.z = 0.0f;
//	vtx.Diffuse = Color(0, 0, 1, 0.6);
//	vtx.Normal = Vector3(0, 0, -1);
//	vtx.TexCoord = Vector2(u[3], v[3]);
//	m_vertices.push_back(vtx);
//
//	// 頂点バッファ生成
//	m_VertexBuffer.Create(m_vertices);
//
//	// シェーダオブジェクト生成
//	m_shader = MeshManager::getShader<CShader>("ulightshader");
//
//	// マテリアル生成
//	MATERIAL	mtrl;
//	mtrl.Ambient = Color(0, 0, 0, 0);
//	mtrl.Diffuse = Color(1, 1, 1, 1);
//	mtrl.Emission = Color(0, 0, 0, 0);
//	mtrl.Specular = Color(0, 0, 0, 0);
//	mtrl.Shiness = 0;
//	mtrl.TextureEnable = TRUE;
//
//	m_Material.Create(mtrl);
//
//	// テクスチャロード
//	bool sts = m_Texture.Load("assets/texture/whitePuff06.png");
//	assert(sts == true);
//	isAlive = true;
//
//	exploding = false;
//	lifeMs = 0.0f;
//	maxSpLifeMs = 300.0f;
//	maxSpRadius = 200.0f;
//	color = Color(1, 0, 0, 0.8f);
//	pos = Vector3(0, 0, 0);
//}

void EffectSystem::init()
{
    exploding = false;
    lifeMs = 0.0f;
    maxSpLifeMs = 300.0f;
    maxSpRadius = 200.0f;
    color = Color(0.5f, 0.7f, 1, 0.8f);
    pos = Vector3(0, 0, 0);
}

static float Clamp01(float t)
{
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

void EffectSystem::StartExplosion(const Vector3& p)
{
    pos = p;

    m_exStart = Time::Get().Now();
    exploding = true;
}

void EffectSystem::update(uint64_t)
{
    if (!exploding) return;

    lifeMs = (float)Time::ElapsedMs(m_exStart);

    if (lifeMs >= maxSpLifeMs)
    {
        exploding = false;
        lifeMs = maxSpLifeMs;
    }
}

void EffectSystem::draw(uint64_t)
{
    if (!exploding) return;

    const float t = Clamp01(lifeMs / maxSpLifeMs);
    const float r = maxSpRadius * t;

    Color c = color;
    c.w = (1.0f - t) * color.w;

    SphereDrawerDraw(r, c, pos.x, pos.y, pos.z);
}

//void EffectSystem::draw(uint64_t dt) {
//
//	if (isAlive) {
//
//		Matrix4x4 mtx = getSRT().GetMatrix();
//		Renderer::SetWorldMatrix(&mtx);
//
//		ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();
//		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//		m_shader->SetGPU();
//		m_VertexBuffer.SetGPU();
//
//		m_Material.SetGPU();
//		m_Texture.SetGPU();
//
//		devicecontext->Draw(4, 0);
//	}
//
//	// explosion（SphereDrawer で球描画）
//	if (exploding) {
//
//		const float t = Clamp01(lifeMs / maxSpLifeMs);
//		const float r = maxSpRadius * t;
//
//		Color c = color;
//		c.w = (1.0f - t) * color.w;
//
//		SphereDrawerDraw(r, c, pos.x, pos.y, pos.z);
//	}
//}

//void EffectSystem::drawbillboard(Camera* camera) {
//
//	Matrix4x4 viewmtx = camera->GetViewMatrix();
//
//	// 転置行列作成
//	Matrix4x4 trasnposemtx = viewmtx.Transpose();
//
//	Matrix4x4 billboardmtx = Matrix4x4::Identity;
//	billboardmtx._11 = trasnposemtx._11;
//	billboardmtx._12 = trasnposemtx._12;
//	billboardmtx._13 = trasnposemtx._13;
//
//	billboardmtx._21 = trasnposemtx._21;
//	billboardmtx._22 = trasnposemtx._22;
//	billboardmtx._23 = trasnposemtx._23;
//
//	billboardmtx._31 = trasnposemtx._31;
//	billboardmtx._32 = trasnposemtx._32;
//	billboardmtx._33 = trasnposemtx._33;
//
//	billboardmtx._41 = m_srt.pos.x;
//	billboardmtx._42 = m_srt.pos.y;
//	billboardmtx._43 = m_srt.pos.z;
//
//	Renderer::SetWorldMatrix(&billboardmtx);
//
//	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();
//	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	m_shader->SetGPU();
//	m_VertexBuffer.SetGPU();
//
//	m_Material.SetGPU();
//	m_Texture.SetGPU();
//
//	devicecontext->Draw(4, 0);
//}

void EffectSystem::dispose() {
}

//void EffectSystem::SetSpeed(float s) { m_speed = s; }
//
//float EffectSystem::GetLifeTime() const {
//	return m_lifeTime;
//}
//
//float EffectSystem::GetMaxLife() const {
//	return m_maxLife;
//}
