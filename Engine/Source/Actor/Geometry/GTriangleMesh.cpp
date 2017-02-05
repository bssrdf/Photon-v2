#include "Actor/Geometry/GTriangleMesh.h"
#include "Actor/Geometry/GTriangle.h"
#include "Core/Primitive/PrimitiveMetadata.h"
#include "Actor/AModel.h"
#include "Actor/Geometry/PrimitiveBuildingMaterial.h"
#include "FileIO/InputPacket.h"

#include <iostream>

namespace ph
{

GTriangleMesh::GTriangleMesh() : 
	Geometry(), 
	m_gTriangles()
{

}

GTriangleMesh::GTriangleMesh(const InputPacket& packet) : 
	Geometry(packet), 
	m_gTriangles()
{
	const std::vector<Vector3R> positions = packet.getVector3rArray("positions");
	const std::vector<Vector3R> texCoords = packet.getVector3rArray("texture-coordinates");
	const std::vector<Vector3R> normals   = packet.getVector3rArray("normals");

	if(!(positions.size() == texCoords.size() && texCoords.size() == normals.size()) || 
	   (positions.empty() || texCoords.empty() || normals.empty()) || 
	   (positions.size() % 3 != 0 || texCoords.size() % 3 != 0 || normals.size() % 3 != 0))
	{
		std::cerr << "warning: at GTriangleMesh::GTriangleMesh(), bad input detected" << std::endl;
		return;
	}

	for(std::size_t i = 0; i < positions.size(); i += 3)
	{
		GTriangle triangle(positions[i], positions[i + 1], positions[i + 2]);
		triangle.setUVWa(texCoords[i]);
		triangle.setUVWb(texCoords[i + 1]);
		triangle.setUVWc(texCoords[i + 2]);
		triangle.setNa(normals[i]);
		triangle.setNb(normals[i + 1]);
		triangle.setNc(normals[i + 2]);
		addTriangle(triangle);
	}
}

GTriangleMesh::~GTriangleMesh()
{

}

void GTriangleMesh::discretize(const PrimitiveBuildingMaterial& data,
                               std::vector<std::unique_ptr<Primitive>>& out_primitives) const
{
	for(const auto& gTriangle : m_gTriangles)
	{
		gTriangle.discretize(data, out_primitives);
	}
}

void GTriangleMesh::addTriangle(const GTriangle gTriangle)
{
	m_gTriangles.push_back(gTriangle);
}

}// end namespace ph