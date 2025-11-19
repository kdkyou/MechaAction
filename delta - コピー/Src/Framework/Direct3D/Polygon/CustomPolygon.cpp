#include "CustomPolygon.h"

// 頂点情報取得
void CustomPolygon::SetVerticesFromPositions(const std::vector<Math::Vector3>& positions)
{
	m_vertices.clear();
	for (const auto& pos : positions)
	{
		Vertex vtx;
		vtx.pos = pos;
		vtx.UV = { 0.0f,0.0f };
		vtx.color = 0xFFFFFFFF;
		m_vertices.push_back(vtx);

	}
}
