#pragma once

#include"../WeaponBase.h"

class Character;

class Sowrd :public WeaponBase
{
public:
	void Init()override;
	void Update()override;

	void DrawUnLit()override;

	void SetParent(std::weak_ptr<Character>_parent);

private:

	std::weak_ptr<Character> m_wpParent;

	std::shared_ptr<KdTrailPolygon> m_spTrail;

	Math::Matrix					m_startMat;
	Math::Matrix					m_endMat;

};