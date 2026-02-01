//---------------------------------------------
// OBB vs OBB 衝突判定
// 2020-11-16 Monday
// Rewrite 2025-12-02 Tuesday
//---------------------------------------------
#pragma once
//#include <d3d11.h>
#include <DirectXMath.h>

//2025-12-02 Tuesday
#include "SimpleMath.h"

//using namespace DirectX;
using namespace DirectX::SimpleMath;

//struct OBB {
//	XMVECTOR m_Pos;              // 位置
//	XMVECTOR m_NormaDirect[3];   // 方向ベクトル
//	FLOAT m_fLength[3];          // 各軸方向の長さ
//};

class COBB
{
public:
	Vector3 m_maxpos;// = XMFLOAT3(0, 0, 0);
	Vector3 m_minpos;// = XMFLOAT3(0, 0, 0);
	Matrix  m_WorldMatrix;

	//FLOAT maxpos;// = XMFLOAT3(0, 0, 0);
	//FLOAT minpos;// = XMFLOAT3(0, 0, 0);
	//Matrix WorldMatrix;
//protected:
	//XMVECTOR m_Pos;              // 位置
	//XMVECTOR m_NormaDirect[3];   // 方向ベクトル
	Vector3 m_Pos;              // 位置
	Vector3 m_NormaDirect[3];   // 方向ベクトル

	FLOAT	m_fLength[3] = {0,0,0};       // 各軸方向の長さ

public:
	
	// 指定軸番号の方向ベクトルを取得

	Vector3 GetDirect(int elem) {
		Vector3 wmaxis;
		switch (elem) {

		case 0:
			wmaxis = { m_WorldMatrix._11,m_WorldMatrix._12 ,m_WorldMatrix._13 };
			break;
		case 1:
			wmaxis = { m_WorldMatrix._21,m_WorldMatrix._22 ,m_WorldMatrix._23 };

			break;
		case 2:
			wmaxis = { m_WorldMatrix._31,m_WorldMatrix._32 ,m_WorldMatrix._33 };

			break;
		}
		
		return wmaxis;
	}
	
	// 指定軸方向の長さを取得
	FLOAT	GetLen_W(int elem) {
		FLOAT localmaxpos=0.0f;// = maxpos[elem];
		FLOAT localminpos=0.0f;// = minpos[elem];

		if (elem == 0) {
			localmaxpos = m_maxpos.x;
			localminpos = m_minpos.x;
		}
		if (elem == 1) {
			localmaxpos = m_maxpos.y;
			localminpos = m_minpos.y;
		}

		if (elem == 2) {
			localmaxpos = m_maxpos.z;
			localminpos = m_minpos.z;
		}

		return (float)fabs(localmaxpos - localminpos) * 0.5f;

	}    
	
	// 位置を取得
	Vector3 GetPos_W() {
		Vector3 localpos;

		localpos = { m_WorldMatrix._41,m_WorldMatrix._42,m_WorldMatrix._43 };
		
		return localpos;
	}; 
};

bool CollisionOBB(COBB &obb1, COBB &obb2);