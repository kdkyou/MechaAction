#pragma once

#include "../../Character/CharacterBase.h"

class FLAC : public CharacterBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawLit()			override;

	void SetThis(const std::shared_ptr<FLAC>& spthis) { m_wpThis = spthis; }

	void OnHit()override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:


	std::weak_ptr<FLAC>				    m_wpThis;

};