#pragma once // (c) 2020 Lukas Brunner

#include <vector>

BRWL_NS

struct VertexData
{
	Vec3 position;
	Vec2 uv;
};

// Primitive shapes are usually centered around origin
namespace ProceduralGeometry
{
	void makeQuad(float xSize, float zSize, std::vector<VertexData>& vertices);
	void makeQuad(float xSize, float zSize, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
	void makeGrid(float xSize, float zSize, int numXSubdiv, int numZSubdiv, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
	void makeCube(float xSize, float ySize, float zSize, std::vector<VertexData>& vertices);
	void makeCube(float xSize, float ySize, float zSize, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
	void makeCylinder(float height, float radius, unsigned int nSegments, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
	void makeSphere(float radius, int nSegmentsLatitude, int nSegmentsLongitude, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
}

BRWL_NS_END