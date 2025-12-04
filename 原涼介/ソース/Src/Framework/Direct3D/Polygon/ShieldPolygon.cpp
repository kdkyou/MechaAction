#include "ShieldPolygon.h"

void ShieldPolygon::SetVertices(const std::vector<Math::Vector3>& positions, const std::vector<Math::Vector2>& uvs)
{
	m_vertices.clear();

	for (size_t i = 0; i < positions.size(); ++i)
	{
		Vertex vtx;
		vtx.pos = positions[i];
		vtx.UV = (i < uvs.size()) ? uvs[i] : Math::Vector2::Zero;
		vtx.color = 0xFFFFFFFF;
		m_vertices.push_back(vtx);
	}

	// Indexは四角錐の4面を構成（中心点を先頭にして三角形構成）
	m_indices = {
		0, 1, 4,  // 三角形1
		1, 2, 4,  // 三角形2
		2, 3, 4,  // 三角形3
		3, 0, 4   // 三角形4
	};
}
