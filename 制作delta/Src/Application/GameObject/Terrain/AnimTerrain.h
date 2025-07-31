#pragma once

class AnimTerrain : public KdGameObject
{
public:
	AnimTerrain() {}
	virtual ~AnimTerrain()		override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void DrawLit()			override;

	void SetPos(const Math::Vector3& vec)override { m_pos = vec; }

	void SetModel(const std::string& path);

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{

	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);

		outJson["path"] = m_modelPath;
	}

private:
	std::shared_ptr<KdModelWork> m_spModel = nullptr;

	std::string m_modelPath = "";

	std::shared_ptr<KdAnimator> m_spAnimator = nullptr;

};