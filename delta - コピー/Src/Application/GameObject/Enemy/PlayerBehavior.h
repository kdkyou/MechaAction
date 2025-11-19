#pragma once

enum class PlayerBehavior
{
	Idle,
	Aggressive,
	Defensive,
	FlankRight,
	FlankLeft,
	Behind,
	Unreadable,

};

PlayerBehavior AnalyzePalayer(const Math::Vector3& playerPos, const Math::Vector3& enemyPos, const Math::Vector3 enemyDir, const float enemyDist,const float enemyViewAng)
{
	Math::Vector3 diff = playerPos - enemyPos;
	float dist = diff.Length();
	float d = diff.Dot(enemyPos);

	//角度求める(でも残念ながらラジアン角)11
	float angle = DirectX::XMConvertToDegrees(acos(d));

	if (dist > enemyDist) return PlayerBehavior::Idle;
	if (angle > enemyViewAng) return PlayerBehavior::Behind;
	if (KeyInput::GetInstance().IsDashingRightFrequently()) return PlayerBehavior::FlankRight;
	if (KeyInput::GetInstance().IsDashingLeftFrequently()) return PlayerBehavior::FlankLeft;
	if (KeyInput::GetInstance().IsAttackingFrequently()) return PlayerBehavior::Aggressive;

	return PlayerBehavior::Unreadable;

}