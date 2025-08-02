#pragma once

class ShieldPolygon :public KdPolygon
{
public:
	ShieldPolygon(){}

	void SetVertices(const std::vector<Math::Vector3>& positions, const std::vector<Math::Vector2>& uvs);

	void ClearVertices() { m_vertices.clear(); }

private:

};