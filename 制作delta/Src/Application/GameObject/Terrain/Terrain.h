#pragma once

class Terrain : public KdGameObject
{
public:
	Terrain() {}
	virtual ~Terrain()		override {}

	void Init()				override;
	void Update()			override;
	void DrawLit()			override;

	void SetMat(const Math::Matrix& mat) { m_mWorld = mat; }

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
	std::shared_ptr<KdModelData> m_spModel = nullptr;

	std::string m_modelPath = "";

};