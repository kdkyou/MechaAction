#include "ParentTerrain.h"

void ParentTerrain::Init()
{
	m_name = "ParentTerrain";
}

void ParentTerrain::Update()
{
	auto parent = m_wpParent.lock();
	if (parent)
	{
		auto pos = parent->GetMatrix().Translation();
		pos.y = 0;
		m_mWorld.Translation(pos);
	}
}

void ParentTerrain::DrawUnLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void ParentTerrain::Editor_ImGui()
{
	DrawTerrain::Editor_ImGui();
}
