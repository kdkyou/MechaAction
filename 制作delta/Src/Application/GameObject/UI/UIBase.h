#pragma once



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
		KdJsonUtility::GetValue(jsonObj, "UITag", &m_uiTag);
		KdJsonUtility::GetValue(jsonObj,"Path",&m_fileName);
		KdJsonUtility::GetValue(jsonObj, "DrawWidth", & m_drawWi);
		KdJsonUtility::GetValue(jsonObj,"DrawHeight",&m_drawHe);
		KdJsonUtility::GetValue(jsonObj, "RectWidth", &m_rectWi);
		KdJsonUtility::GetValue(jsonObj, "RectHeight", &m_rectHe);
		KdJsonUtility::GetArray(jsonObj, "Color", &m_color.x,4);
		KdJsonUtility::GetArray(jsonObj, "Pivot", &m_pivot.x,2);
		KdJsonUtility::GetValue(jsonObj, "TimeRimit", &m_isTimeRimit);
		KdJsonUtility::GetValue(jsonObj, "AliveTime", &m_aliveTime);

		if (m_fileName != "")
		{
			SetTexture(m_fileName);
		}
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);
		outJson["UITag"] = m_uiTag;
		outJson["Path"] = m_fileName;
		outJson["DrawWidth"] = m_drawWi;
		outJson["DrawHeight"] = m_drawHe;
		outJson["RectWidth"] = m_rectWi;
		outJson["RectHeight"] = m_rectHe;
		outJson["Color"] = KdJsonUtility::CreateArray(&m_color.x, 4);
		outJson["Pivot"] = KdJsonUtility::CreateArray(&m_pivot.x, 2);
		outJson["TimeRimit"] = m_isTimeRimit;
		outJson["AliveTime"] = m_aliveTime;

	}

	void SetTexture(const std::string& path);

protected:

	std::string				   m_uiTag = "None";

	std::shared_ptr<KdTexture> m_spTex = nullptr;

	bool					   m_isTimeRimit = false;
	float					   m_aliveTime = 0.0f;

	Math::Rectangle			   m_rect;

	int						   m_rectX =0;
	int						   m_rectY =0;
	int						   m_drawWi=0;
	int						   m_drawHe=0;
	int						   m_rectWi=0;
	int						   m_rectHe=0;

	std::string				   m_fileName;

	Math::Color				   m_color = { 1,1,1,1 };
	Math::Vector2			   m_pivot = { 0.5f,0.5f };

};