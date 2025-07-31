#pragma once

#define UIFILEPATH "Asset/Textures/UI/"

class UIBase :public KdGameObject
{
public:

	void Init()override;

	void DrawSprite()override;

	virtual void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)
	{
		KdGameObject::Deserialize(jsonObj);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const
	{
		KdGameObject::Serialize(outJson);
	}


protected:

	std::shared_ptr<KdTexture> m_spTex;

	Math::Rectangle			   m_rect;

};