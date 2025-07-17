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

private:
	std::shared_ptr<KdModelWork> m_spModel = nullptr;

};