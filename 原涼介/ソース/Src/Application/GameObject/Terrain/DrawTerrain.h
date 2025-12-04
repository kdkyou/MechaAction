#pragma once

class DrawTerrain :public KdGameObject
{
public:
	DrawTerrain() {}
	virtual ~DrawTerrain()		override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void DrawLit()			override;
	
	void SetPos(const Math::Vector3& vec)override { m_pos = vec; }

	void SetModel(const std::string& path);
	void CreateAnimator();

	virtual void Editor_ImGui() override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		KdGameObject::Deserialize(jsonObj);

		KdJsonUtility::GetValue(jsonObj, "ModelPath", &m_modelPath);
		KdJsonUtility::GetValue(jsonObj, "AnimPath", &m_animPath);
		KdJsonUtility::GetArray(jsonObj, "Emmisive", &m_emmisive.x, 3);
		KdJsonUtility::GetValue(jsonObj, "AnimSpeed", &m_animSpeed);
		KdJsonUtility::GetArray(jsonObj, "ModelColor", &m_modelColor.x, 4);
		KdJsonUtility::GetValue(jsonObj, "SoundPath", &m_soundPath);
		
		SetModel(m_modelPath);
		if (m_animPath != "") {
			CreateAnimator();
			AnimationPlay(m_animPath);
		}

		if (jsonObj.contains("Points"))
		{
			for (auto& data : jsonObj["Points"])
			{
				PointLight p;
				KdJsonUtility::GetArray(data, "Pos", &p.Pos.x, 3);
				KdJsonUtility::GetValue(data, "Radius", &p.Radius);
				KdJsonUtility::GetArray(data, "Color", &p.Color.x,3);
				p.IsBright = 1;
				m_points.push_back(p);

			}
		}
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);

		outJson["ModelPath"] = m_modelPath;
		outJson["AnimPath"] = m_animPath;
		outJson["Emmisive"] = KdJsonUtility::CreateArray(&m_emmisive.x, 3);
		outJson["AnimSpeed"] = m_animSpeed;
		outJson["SoundPath"] = m_soundPath;
		outJson["ModelColor"] = KdJsonUtility::CreateArray(&m_modelColor.x, 4);

		nlohmann::json pointsArray = nlohmann::json::array();

		for (auto& obj : m_points)
		{
			{
				nlohmann::json pointJson;
				pointJson["Pos"] =KdJsonUtility::CreateArray(&obj.Pos.x,3);
				pointJson["Radius"] = obj.Radius;
				pointJson["Color"] = KdJsonUtility::CreateArray(&obj.Color.x,3);
				pointsArray.push_back(pointJson);
			}
		}

		outJson["Points"] = pointsArray;


	}

protected:

	void AnimationPlay(const std::string& path);

	std::shared_ptr<KdModelWork> m_spModel = nullptr;

	std::string					m_modelPath = "";

	std::shared_ptr<KdAnimator> m_spAnimator = nullptr;

	std::string					m_animPath = "";

	std::string					m_soundPath = "";

	Math::Color					m_modelColor = kWhiteColor;

	float						m_animSpeed = 0.0f;

	Math::Vector3				m_emmisive = Math::Vector3::Zero;
	
	std::vector<PointLight>		m_points;



};