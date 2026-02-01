cpp system\camera.cpp
#include "commonTypes.h"
#include "renderer.h"
#include "camera.h"
#include "../application.h"
#include "../gameobject/gameobject.h" // フォロー対象の SRT を取得するため

void Camera::Init()
{
	//視点
	m_position = Vector3(0, 50, -80);
	//注視点
	m_lookat = Vector3(0, 0, 0);
}

void Camera::Dispose()
{

}

void Camera::Update()
{
	// モード毎の挙動
	switch (m_mode)
	{
	case CameraMode::Follow:
		if (m_followTarget) {
			Vector3 targetPos = m_followTarget->getSRT().pos;
			Vector3 desiredPos = targetPos + m_followOffset;
			// シンプルな lerp
			m_position = m_position + (desiredPos - m_position) * m_followSmooth;
			m_lookat = targetPos;
		}
		break;

	case CameraMode::FirstPerson:
		if (m_followTarget) {
			SRT srt = m_followTarget->getSRT();
			// 前方ベクトルは Y 回転のみで算出（既存コードと整合）
			float yaw = srt.rot.y;
			Vector3 forward = Vector3(-sinf(yaw), 0.0f, -cosf(yaw));
			// 目の位置をセット
			m_position = srt.pos + m_firstPersonOffset;
			m_lookat = m_position + forward * 10.0f;
		}
		break;

	case CameraMode::TopDown:
		if (m_followTarget) {
			Vector3 targetPos = m_followTarget->getSRT().pos;
			m_position = Vector3(targetPos.x, targetPos.y + m_topDownHeight, targetPos.z);
			m_lookat = targetPos;
		}
		break;

	case CameraMode::Fixed:
		// 変更しない
		break;

	case CameraMode::Free:
	default:
		// 外部で自由に位置を操作する想定（Update内では何もしない）
		break;
	}
}

void Camera::Draw()
{
	// ビュー変換行列作成
	m_viewmtx =
		DirectX::XMMatrixLookAtLH(
			m_position,
			m_lookat,
			m_up);

	Renderer::SetViewMatrix(&m_viewmtx);

	// プロジェクション行列の生成
	constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角

	float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
	float nearPlane = 1.0f;       // ニアクリップ
	float farPlane = 10000.0f;      // ファークリップ

	// プロジェクション行列の生成
	m_projmtx =
		DirectX::XMMatrixPerspectiveFovLH(
			fieldOfView,
			aspectRatio,
			nearPlane,
			farPlane);

	Renderer::SetProjectionMatrix(&m_projmtx);
}