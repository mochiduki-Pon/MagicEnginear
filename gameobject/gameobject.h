#pragma once
#include <cstdint>

#include	"../system/transform.h"
#include	"../system/IScene.h"

class gameobject {
public:
	gameobject() = default;
	// ★ ここで基底クラスのメンバを初期化する
	explicit gameobject(IScene* currentscene)
		: m_ownerscene(currentscene){}

	virtual ~gameobject() = default;
	virtual void update(uint64_t delta) = 0;
	virtual void draw(uint64_t delta) = 0;
	virtual void init() = 0;
	virtual void dispose() = 0;

	SRT getSRT() {
		return m_srt;
	}

	void	setSRT(const SRT& srt) {
		m_srt = srt;
	}

	void	setPosition(const Vector3& pos) {
		m_srt.pos = pos;
	}

protected:
	SRT		m_srt{};
	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
};

