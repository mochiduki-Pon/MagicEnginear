#include	"tower.h"
#include	"../system/CMaterial.h"
#include	"../system/CMeshRenderer.h"
#include	"../system/meshmanager.h"
#include	"../scene/TutorialScene.h"
#include "../system/CStaticMesh.h"
#include "../system/CStaticMeshRenderer.h"
#include "../system/Cooldown.h"
#include "../system/CylinderDrawer.h"

void tower::init() {

	m_staticmeshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("Heart_Brilliant.fbx");
	m_staticmeshshader = MeshManager::getShader <CShader>("lightshader");

	float rad = DirectX::XM_PI / 180.0f;
	m_srt.pos = Vector3(0, 25, -100);
	//m_srt.pos = Vector3(0, 25, 0);
	m_srt.scale = Vector3(0.9f, 0.9f, 0.9f);
	m_srt.rot = Vector3(90 * rad, 0, 0);
}

//当り部分
Call::Collision::BoundingCylinder tower::GetCylinder() const
{
	Call::Collision::BoundingCylinder c{};

	c.bottom = m_srt.pos;
	c.top = m_srt.pos + Vector3(0.0f, m_colHeight, 0.0f);
	c.radius = m_colRadius;

	return c;
}

void tower::Reboot()
{
	m_cantAct = false;
}

void tower::OnDMF()
{
	m_vstate = VisualState::DMF;
	m_damageRemainMs = DAMAGE_FLASH_MS;
}

bool tower::UnderAtack(int amount)
{
	if (m_sts == Status::Dead) return false;
	if (amount <= 0) return false;

	const int before = m_hp;

	m_hp -= amount;
	if (m_hp <= 0) { m_hp = 0; m_sts = Status::Dead; }

	m_damageRemainMs = DAMAGE_FLASH_MS;
	m_vstate = VisualState::DMF;

	std::cout << "[TowerDamage] -" << amount
		<< " HP " << before << " -> " << m_hp
		<< std::endl;

	return true;
}

bool tower::IsEnd() const
{
	return m_sts == Status::Dead;
}

void tower::update(uint64_t dt)
{
	if (m_sts == Status::Dead)
		return;

	// 地面追従
	field* field = ((TutorialScene*)m_ownerscene)->getfield();
	float h = field->GetHeight2(m_srt.pos);
	m_srt.pos.y = h;

	if (m_hp <= m_maxHp / 2 && m_vstate != VisualState::DMF)
	{
		m_vstate = VisualState::Danger;
	}

	// フラッシュ時間更新
	if (m_damageRemainMs > 0)
	{
		if (m_damageRemainMs > dt)
			m_damageRemainMs -= dt;
		else
			m_damageRemainMs = 0;

		m_vstate = VisualState::DMF;
	}
	else
	{
		// フラッシュしてない時だけ HP を見る
		if (m_hp <= m_maxHp / 2)
			m_vstate = VisualState::Danger;
		else
			m_vstate = VisualState::Normal;
	}
}

void tower::ApplyDrawColor() const
{
	if (m_vstate == VisualState::DMF && m_damageRemainMs > 0)
	{
		float t =
			static_cast<float>(m_damageRemainMs) /
			static_cast<float>(DAMAGE_FLASH_MS);

		float flash =
			1.0f + (HIT_FLASH_MAX_MULTIPLIER - 1.0f) * t;

		m_staticmeshrenderer->Draw(Color(flash, flash, flash, 1.0f));
	}
	else
	{
		m_staticmeshrenderer->Draw(Color(1.0f, 0.0f, 0.3f, 0.5f));
	}
}

//void tower::draw(uint64_t dt)
//{
//	static bool inited = false;
//	if (!inited) { CylinderDrawerInit(); inited = true; }
//
//	const auto c = GetCylinder();
//	const float height = (c.top - c.bottom).Length();
//
//	//Matrix4x4 mtx = m_srt.GetMatrix();
//	SRT drawSrt = m_srt;   // ← 値コピー
//	drawSrt.pos.y += m_OffsetY;
//	Matrix4x4 mtx = drawSrt.GetMatrix();
//	Renderer::SetWorldMatrix(&mtx);
//
//	m_staticmeshshader->SetGPU();
//
//	ApplyDrawColor();
//
//	//CylinderDrawerDraw(
//	//	c.radius,
//	//	height,
//	//	Color(0.0f, 0.7f, 1.0f, 0.08f),
//	//	c.bottom.x, c.bottom.y, c.bottom.z);
//
//}

void tower::draw(uint64_t)
{
	if (m_sts == Status::Dead)
		return;
		static bool inited = false;
	if (!inited) { CylinderDrawerInit(); inited = true; }

	const auto c = GetCylinder();
	const float height = (c.top - c.bottom).Length();

	//Matrix4x4 mtx = m_srt.GetMatrix();
	SRT drawSrt = m_srt;   // ← 値コピー
	drawSrt.pos.y += m_OffsetY;
	Matrix4x4 mtx = drawSrt.GetMatrix();
	Renderer::SetWorldMatrix(&mtx);
	m_staticmeshshader->SetGPU();

	switch (m_vstate)
	{
	case VisualState::DMF:
	{
		float t = float(m_damageRemainMs) / DAMAGE_FLASH_MS;
		float flash = 1.0f + (HIT_FLASH_MAX_MULTIPLIER - 1.0f) * t;
		m_staticmeshrenderer->Draw(Color(flash, flash, flash, 1.0f));
		break;
	}
	case VisualState::Danger:{
		const Color DiffuseColor = Color(1.2f, 1.2f, 0.15f, 0.35f);
		m_staticmeshrenderer->Draw(DiffuseColor);
		break;}
	default: {
		Color DiffuseColor = Color(1.0f, 0.0f, 0.3f, 0.5f); m_staticmeshrenderer->Draw(DiffuseColor);//0.3悪くない
		break;}
	}

	CylinderDrawerDraw(
	c.radius,
	height,
	//Color(0.0f, 0.7f, 1.0f, 0.08f),
	Color(0.7f, 1.0f, 1.0f, 0.08f),
	c.bottom.x, c.bottom.y, c.bottom.z);
}

void tower::dispose() {

}