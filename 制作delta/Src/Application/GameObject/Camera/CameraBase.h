#pragma once

#include"GamePad.h"

class CharacterBase;

class CameraBase : public KdGameObject
{
public:
	CameraBase()						{}
	virtual ~CameraBase()	override	{}

	void Init()				override;

	// カメラに3Dリスナーを置く
	void PostUpdate()			override;
	void PreDraw()			override;


	void SetLook(const std::weak_ptr<KdGameObject>& target);
	void SetTarget(const std::weak_ptr<CharacterBase>& target);
	void SetLockTarget(const std::weak_ptr<CharacterBase>& rock);

	// 「絶対変更しません！見るだけ！」な書き方
	const std::shared_ptr<KdCamera>& GetCamera() const
	{
		return m_spCamera;
	}

	// 「中身弄るかもね」な書き方
	std::shared_ptr<KdCamera> WorkCamera() const
	{
		return m_spCamera;
	}

	const Math::Matrix GetRotationMatrix()const
	{
		return Math::Matrix::CreateFromYawPitchRoll(
		       DirectX::XMConvertToRadians(m_DegAng.y),
		       DirectX::XMConvertToRadians(m_DegAng.x),
		       DirectX::XMConvertToRadians(m_DegAng.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_DegAng.y));
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	const std::string& GetName() { return m_name; }

	const Math::Vector3& GetDeg() { return m_DegAng; }

	void SetDeg(const Math::Vector3& ang) { m_DegAng = ang; }

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;


protected:

	std::string									m_name;

	// カメラ回転用角度
	Math::Vector3								m_DegAng = Math::Vector3::Zero;
	Math::Vector3								m_localPos = Math::Vector3::Zero;

	void UpdateRotateByMouse();

	std::shared_ptr<KdCamera>					m_spCamera		= nullptr;
	std::weak_ptr<KdGameObject>						m_wpLook;
	std::weak_ptr<CharacterBase>					m_wpTarget;
	std::weak_ptr<CharacterBase>					m_wpLockTarget;

	
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Matrix								m_mLocalPos		= Math::Matrix::Identity;
	Math::Matrix								m_mRotation		= Math::Matrix::Identity;
	
	// カメラ回転用マウス座標の差分
	POINT										m_FixMousePos{};

	Math::Vector2								m_mouseSpeed = { 10.0f,10.0f };


};