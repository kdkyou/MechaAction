#pragma once

#define UIFILEPATH "Asset/Textures/UI/"

class UIBase :public KdGameObject
{
public:

	const std::string& GetUIByTag()const { return m_uiTag; }

	void Init()override;

	void DrawSprite()override;

	virtual void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		KdGameObject::Deserialize(jsonObj);
		KdJsonUtility::GetValue(jsonObj, "Tag", &m_uiTag);
		KdJsonUtility::GetValue(jsonObj,"Path",&m_fileName);
		KdJsonUtility::GetValue(jsonObj, "DrawWidth", & m_drawWi);
		KdJsonUtility::GetValue(jsonObj,"DrawHeight",&m_drawHe);
		KdJsonUtility::GetValue(jsonObj, "RectWidth", &m_rectWi);
		KdJsonUtility::GetValue(jsonObj, "RectHeight", &m_rectHe);

		if (m_fileName != "")
		{
			SetTexture(m_fileName);
		}
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);
		outJson["Tag"] = m_uiTag;
		outJson["Path"] = m_fileName;
		outJson["DrawWidth"] = m_drawWi;
		outJson["DrawHeight"] = m_drawHe;
		outJson["RectWidth"] = m_rectWi;
		outJson["RectHeight"] = m_rectHe;
	}

	void SetTexture(const std::string& path);

protected:

	std::string				   m_uiTag = "None";

	std::shared_ptr<KdTexture> m_spTex;


	Math::Rectangle			   m_rect;

	int						   m_rectX;
	int						   m_rectY;
	int						   m_drawWi;
	int						   m_drawHe;
	int						   m_rectWi;
	int						   m_rectHe;

	std::string				   m_fileName;

	Math::Color				   m_color = { 1,1,1,1 };
	Math::Vector2			   m_pivot = { 0.5f,0.5f };

};