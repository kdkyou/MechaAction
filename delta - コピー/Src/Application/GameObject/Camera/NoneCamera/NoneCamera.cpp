#include"NoneCamera.h"

void NoneCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90)) * Math::Matrix::CreateTranslation(0, 50.0f, 0.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_name = "None";

	ShowCursor(false);
}

void NoneCamera::Update()
{
	if(EditorData::GetInstance().m_editorMode)
	{
		auto& key = KeyInput::GetInstance().GetKeyboardState();
		auto& mouse = KeyInput::GetInstance().GetMouseState();
		if (mouse.rightButton)
		{
			Math::Vector3 direction = {};
				if (key.W)
				{
					direction.z += 1.0f;
				}
				if (key.A)
				{
					direction.x -= 1.0f;
				}
				if (key.S)
				{
					direction.z -= 1.0f;
				}
				if (key.D)
				{
					direction.x += 1.0f;
				}

			auto vec = direction.TransformNormal(direction, GetRotationYMatrix());
			vec.Normalize();

			m_pos += vec * m_movePow * KdFPSController::GetInstance().GetDeltaTime();

			UpdateRotateByMouse();
			m_mRotation = GetRotationMatrix();

			m_mWorld = m_mLocalPos * m_mRotation * Math::Matrix::CreateTranslation(m_pos);

		}
	}
}

void NoneCamera::PostUpdate()
{
	//if (!EditorData::GetInstance().m_editorMode)
	{
		m_mWorld = Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);
	}

	CameraBase::PostUpdate();
}

void NoneCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();
	ImGui::SliderFloat("MovePow", &m_movePow, 0.0f,100.0f);
}

void NoneCamera::Deserialize(const nlohmann::json& jsonObj)
{
	KdJsonUtility::GetArray(jsonObj, "Pos", &m_pos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Rot", &m_rot.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Scale", &m_scale.x, 3);
}

void NoneCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);
}
