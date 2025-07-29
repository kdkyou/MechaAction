#include "Effect.h"

void Effect::Init()
{

}

void Effect::SetTexture(const std::string& path)
{
	if (m_spTex)
	{
		m_spTex = KdAssets::Instance().m_textures.GetData(path);
	}
	if (m_spPoly)
	{
		m_spPoly->SetMaterial(KdAssets::Instance().m_textures.GetData(path));
	}
}



void Effect::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);
}

void Effect::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);
}
