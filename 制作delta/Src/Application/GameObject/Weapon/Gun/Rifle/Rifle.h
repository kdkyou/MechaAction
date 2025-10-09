#pragma once

#include"../GunBase.h"

class CharacterBase;

class Rifle :public GunBase
{
public :
	void Init()override;

	void PreUpdate()override;
	void Update()override;


	void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	void Trigger() override;
	void Shot() override;
	void OnTrigger()override;

};