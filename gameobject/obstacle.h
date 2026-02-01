#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"

class obstacle : public gameobject {

public:
	obstacle(IScene* currentscene)
		: m_meshrenderer(nullptr),
		m_ownerscene(currentscene) {
	}

	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	float getwidth() const {
		return m_width;
	}
	float getheight() const {
		return m_height;
	}
	float getdepth() const {
		return m_depth;
	}

private:
	CStaticMesh*			m_mesh{};
	CStaticMeshRenderer*	m_meshrenderer{};
	CShader*				m_shader{};

	float m_width{};
	float m_height{};
	float m_depth{};

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
};