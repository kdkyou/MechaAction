#pragma once

#include "../UIBase.h"

class NumberUI :public UIBase
{
public:

	enum class AlignType
	{
		Left,
		Right,
		Center,
	};

	void Init()override;

	void Update()override;

	void PostUpdate()override;

	void DrawSprite()override;

	void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)override;
	

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;
	

	void SetNumber(int num) { m_number = num; }
	void ChangeColor(const Math::Color& color) { m_color = color; }
	void SetDigit(int digit) { m_maxDigit = digit; }
	void SetAlign(AlignType align) { m_align = align; }
	void SetSpacing(float spacing) { m_spacing = spacing; }

private:

	void DrawNumber();

	int m_number = 0;
	int m_maxDigit = 3;
	float m_spacing = 32.0f;
	AlignType m_align = AlignType::Right;

	bool				 m_isPadding = false;

	static constexpr int kDigitWidth = 32;
	static constexpr int kDigitHeight = 32;

	 
};