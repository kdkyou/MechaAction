#pragma once

class Terrain : public KdGameObject
{
public:
	Terrain() {}
	virtual ~Terrain()		override {}

	void Init()				override;
	void Update()			override;
	void GenerateDepthMapFromLight()override;
	void DrawLit()			override;
	void DrawUnLit()		override;
	void DrawBright()		override;

	void SetPos(const Math::Vector3& vec)override { m_pos = vec; }

	void SetModel(const std::string& path);

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		KdGameObject::Deserialize(jsonObj);
		KdJsonUtility::GetValue(jsonObj, "path", &m_modelPath);

		KdJsonUtility::GetValue(jsonObj, "Generate", &m_isGenerate);
		KdJsonUtility::GetValue(jsonObj, "Lit", &m_isLit);
		KdJsonUtility::GetValue(jsonObj, "UnLit", &m_isUnLit);
		KdJsonUtility::GetValue(jsonObj, "Bright", &m_isBright);
		SetModel(m_modelPath);

		auto angle = KdToRadians * m_rot;
		m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);

		outJson["path"] = m_modelPath;
		outJson["Generate"] =m_isGenerate;
		outJson["Lit"] = m_isLit;
		outJson["UnLit"] = m_isUnLit;
		outJson["Bright"] =m_isBright;	}

private:
	std::shared_ptr<KdModelData> m_spModel = nullptr;

	std::string m_modelPath = "";

	bool m_isGenerate = true;
	bool m_isLit = true;
	bool m_isUnLit =false;
	bool m_isBright = false;
};