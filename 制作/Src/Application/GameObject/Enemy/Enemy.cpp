#include"Enemy.h"

void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Grint/grint.gltf");

	}
}
