#include "WeaponBase.h"



void WeaponBase::PreUpdate()
{
	if (m_wpParent.expired() == true) {
		m_isExpired = true;
	}
}

void WeaponBase::SetParent(std::shared_ptr<CharacterBase> _parent)
{
	m_wpParent = _parent;
}

void WeaponBase::SetModel(const std::string& path) {

	if (path == "") { return; }
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(path);
		m_modelPath = path;
	}
}

void WeaponBase::SetModelData(const std::string& path)
{
	if (path == "") { return; }

	if (!m_spModelData)
	{
		m_spModelData = KdAssets::Instance().m_modeldatas.GetData(path);
	}
}

void WeaponBase::SetAttachPath(const std::string& attachPath)
{
	m_attachPath = attachPath;

}

void WeaponBase::DrawLit()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld,kWhiteColor,m_emissive);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld, kWhiteColor, m_emissive);
	}

}

void WeaponBase::GenerateDepthMapFromLight()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, kWhiteColor, m_emissive);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld, kWhiteColor, m_emissive);
	}
}

void WeaponBase::CheckTrigger()
{
	auto spParent = m_wpParent.lock();

	m_nowTrigger = 0;

	if (spParent) {

		if (spParent->IsLeftAttack())
		{
			m_nowTrigger |= TriggerType::LeftHand;
		}

		if (spParent->IsLeftShoulderAttack())
		{
			m_nowTrigger |= TriggerType::LeftShoulder;
		}

		if (spParent->IsRightAttack())
		{
			m_nowTrigger |= TriggerType::RightHand;
		}

		if (spParent->IsRightShoudlerAttack())
		{
			m_nowTrigger |= TriggerType::RightShoulder;
		}
	}
}

void WeaponBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

	ImGui::DragInt((const char*)u8"判定回数", &m_attackNum);
	static std::string attach;
	ImGui::InputText((const char*)u8"取付位置", &attach);
	if (attach !="")
	{
		if(ImGui::Button((const char*)u8"位置適応")) {
			m_attachPath = attach;
		}
	}
}

void WeaponBase::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);

	KdJsonUtility::GetValue(jsonObj,"ModelPath",&m_modelPath);
	KdJsonUtility::GetValue(jsonObj,"AttackNum",&m_attackNum);
	KdJsonUtility::GetValue(jsonObj,"AttachPath",&m_attachPath);

	SetModel(m_modelPath);
}

void WeaponBase::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);

	outJson["ModelPath"] = m_modelPath;
	outJson["AttackNum"] = m_attackNum;
	outJson["AttachPath"] = m_attachPath;

}


