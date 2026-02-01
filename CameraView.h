cpp system\camera.h
#pragma once

#include	"commontypes.h"
#include	"renderer.h"
#include	"CPolar3D.h"
#include	"../application.h"

class gameobject;

struct CameraView {
	Vector3 position;
	Vector3 lookat;
	Vector3 up;
};

enum class CameraMode {
	Free,
	Follow,
	FirstPerson,
	TopDown,
	Fixed
};

class Camera {
protected:
	Vector3	m_position = Vector3(0.0f, 0.0f, 0.0f);	// カメラ位置
	Vector3	m_lookat{};				// 注視点
	Vector3	m_up = { 0,1,0 };		// アップベクトル			
	Matrix4x4	m_viewmtx{};			// ビュー変換行列
	Matrix4x4   m_projmtx{};			// プロジェクション行列

	// 追従用
	gameobject* m_followTarget = nullptr;
	Vector3     m_followOffset = Vector3(0, 50, -80);
	float       m_followSmooth = 0.12f; // 0..1 の lerp 値（大きいほど速く追従）

	// モード
	CameraMode  m_mode = CameraMode::Free;

	// FirstPerson / TopDown 用パラメータ
	Vector3     m_firstPersonOffset = Vector3(0.0f, 10.0f, 0.0f);
	float       m_topDownHeight = 200.0f;

public:
	virtual ~Camera(){}
	Camera() = default;

	Camera(Vector3 pos, Vector3 lookat,Vector3 up)
		:m_position(pos),m_lookat(lookat),m_up(up) {}

	void Init();
	void Dispose();
	void Update();
	void Draw();
	void SetPosition(const Vector3& position) { m_position = position; }
	void SetLookat(const Vector3& position) { m_lookat = position; }
	void SetUP(const Vector3& up) { m_up = up; }

	// 追従API
	void SetFollowTarget(gameobject* target, const Vector3& offset = Vector3(0, 50, -80), float smooth = 0.12f) {
		m_followTarget = target;
		m_followOffset = offset;
		m_followSmooth = smooth;
	}

	void ClearFollow() { m_followTarget = nullptr; }

	// モード操作
	void SetMode(CameraMode mode) { m_mode = mode; }
	CameraMode GetMode() const { return m_mode; }

	// FirstPerson/TopDown 設定
	void SetFirstPersonOffset(const Vector3& offs) { m_firstPersonOffset = offs; }
	void SetTopDownHeight(float h) { m_topDownHeight = h; }

	// 現在の視点を書き出す（シリアライズ用に呼び出し元へ渡せる）
	CameraView GetView() const { return CameraView{ m_position, m_lookat, m_up }; }
	void SetView(const CameraView& v) { m_position = v.position; m_lookat = v.lookat; m_up = v.up; }

	Matrix4x4 GetViewMatrix() const { return m_viewmtx; }
	Matrix4x4 GetProjMatrix() const { return m_projmtx; }

	Vector3 GetPosition() const{
		return m_position;
	}
	Vector3 GetLookat() const{
		return m_lookat;
	}
	Vector3 GetUP() const {
		return m_up;
	}
};