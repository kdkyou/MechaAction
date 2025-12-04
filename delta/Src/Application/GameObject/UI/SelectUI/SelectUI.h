#pragma once

#include "../UIBase.h"

class SelectUI :public UIBase
{
public:

	SelectUI(){}
	~SelectUI(){}

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawSprite()override;

	void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)override;


	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	enum MoveType
	{
		None,
		TopDown,
		LeftRight,
		Max,
	};

	struct SelectData
	{
		std::string path;	//パス
		std::shared_ptr<KdTexture> tex = nullptr;		//テクスチャ
		Math::Vector2 pos = {}; //座標
		Math::Color color = {1,1,1,1}; //色
		int sceneType;	   //シーンの選択
	};

	MoveType m_type = MoveType::TopDown;

	int m_subscript = 0;

	Math::Vector2 m_selectedPos;
	UINT		  m_selectedScene = 0;

	std::list<std::shared_ptr<SelectData>> m_dataes;

	bool m_isChangeScene = false;

	float m_alpha = 1.0f;

	float m_WorstAlpha = 0.4f;

	float m_time = 0.5f;
	float m_durationTime = 0.0f;
	int   m_plamai = 1;
};