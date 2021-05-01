#include "ProceduralGeometry.h"

BRWL_NS


namespace ProceduralGeometry
{
	void makeQuad(float xSize, float ySize, std::vector<VertexData>& vertices)
	{
		xSize *= 0.5f;
		ySize *= 0.5f;

		vertices.resize(6);
		vertices[0] = { {-xSize,  ySize, 0}, {0.0f, 0.0f} };
		vertices[1] = { { xSize,  ySize, 0}, {1.0f, 0.0f} };
		vertices[2] = { {-xSize, -ySize, 0}, {0.0f, 1.0f} };
		vertices[3] = { {-xSize, -ySize, 0}, {0.0f, 1.0f} };
		vertices[4] = { { xSize,  ySize, 0}, {1.0f, 0.0f} };
		vertices[5] = { { xSize, -ySize, 0}, {1.0f, 1.0f} };
	}

	void makeQuad(float xSize, float zSize, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices)
	{
		xSize *= 0.5f;
		zSize *= 0.5f;

		vertices.resize(4);
		vertices[0] = { {-xSize,  zSize, 0}, {0.0f, 0.0f} };
		vertices[1] = { { xSize,  zSize, 0}, {1.0f, 0.0f} };
		vertices[2] = { { xSize, -zSize, 0}, {1.0f, 1.0f} };
		vertices[3] = { {-xSize, -zSize, 0}, {0.0f, 1.0f} };

		indices.resize(6);
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
	}

	void makeGrid(float xSize, float zSize, int numXSubdiv, int numZSubdiv, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices)
	{
		BRWL_EXCEPTION(numZSubdiv > 0 && numXSubdiv > 0, nullptr);
		float xStep = xSize / numXSubdiv;
		float ZStep = zSize / numZSubdiv;
		float startX = -xSize * 0.5f;
		float startZ = -zSize * 0.5f;
		vertices.resize((numXSubdiv + 1) * (numZSubdiv + 1));
		vertices.resize(6 * numXSubdiv * numZSubdiv);
		for (int z = 0; z < numZSubdiv; ++z)
		{
			for (int x = 0; x < numXSubdiv; ++x) {
				vertices[z * (numXSubdiv + 1) + x] = { {startX + x * xStep, 0,  startZ + z * ZStep}, {(float)x / (float)numXSubdiv, (float)z / (float)numZSubdiv} };
				unsigned int base = 6 * (z * (numXSubdiv)+x);
				indices[base + 0] = z * (numXSubdiv + 1) + x;
				indices[base + 1] = z * (numXSubdiv + 1) + x + 1;
				indices[base + 2] = (z + 1) * (numXSubdiv + 1) + x + 1;
				indices[base + 3] = z * (numXSubdiv + 1) + x;
				indices[base + 4] = (z + 1) * (numXSubdiv + 1) + x + 1;
				indices[base + 5] = (z + 1) * (numXSubdiv + 1) + x;
			}

			vertices[z * (numXSubdiv + 1) + numXSubdiv] = { {startX + numXSubdiv * xStep, 0,  startZ + z * ZStep}, {1.0f, (float)z / (float)numZSubdiv} };
		}

		vertices[numZSubdiv * (numXSubdiv + 1) + numXSubdiv + 1] = { {startX + numXSubdiv * xStep, 0,  startZ + numZSubdiv * ZStep}, {1.0f, 1.0f} };
	}

	void makeCube(float xSize, float ySize, float zSize, std::vector<VertexData>& vertices)
	{
		xSize *= 0.5f;
		ySize *= 0.5f;
		zSize *= 0.5f;

		vertices.resize(36);
		vertices[0] = { { -xSize, -ySize,  zSize}, {0, 0} };
		vertices[1] = { {  xSize, -ySize,  zSize}, {1, 0} };
		vertices[2] = { {  xSize,  ySize,  zSize}, {1, 1} };
		vertices[3] = { { -xSize, -ySize,  zSize}, {0, 0} };
		vertices[4] = { {  xSize,  ySize,  zSize}, {1, 1} };
		vertices[5] = { { -xSize,  ySize,  zSize}, {0, 1} };
		vertices[6] = { { -xSize, -ySize, -zSize}, {1, 0} };
		vertices[7] = { {  xSize,  ySize, -zSize}, {0, 1} };
		vertices[8] = { {  xSize, -ySize, -zSize}, {0, 0} };
		vertices[9] = { { -xSize, -ySize, -zSize}, {1, 0} };
		vertices[10] = { { -xSize,  ySize, -zSize}, {1, 1} };
		vertices[11] = { {  xSize,  ySize, -zSize}, {0, 1} };
		vertices[12] = { { -xSize, -ySize, -zSize}, {0, 0} };
		vertices[13] = { { -xSize, -ySize,  zSize}, {1, 0} };
		vertices[14] = { { -xSize,  ySize, -zSize}, {0, 1} };
		vertices[15] = { { -xSize, -ySize,  zSize}, {1, 0} };
		vertices[16] = { { -xSize,  ySize,  zSize}, {1, 1} };
		vertices[17] = { { -xSize,  ySize, -zSize}, {0, 1} };
		vertices[18] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[19] = { {  xSize,  ySize,  zSize}, {0, 1} };
		vertices[20] = { {  xSize, -ySize,  zSize}, {0, 0} };
		vertices[21] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[22] = { {  xSize,  ySize, -zSize}, {1, 1} };
		vertices[23] = { {  xSize,  ySize,  zSize}, {0, 1} };
		vertices[24] = { { -xSize,  ySize,  zSize}, {0, 0} };
		vertices[25] = { {  xSize,  ySize,  zSize}, {1, 0} };
		vertices[26] = { {  xSize,  ySize, -zSize}, {1, 1} };
		vertices[27] = { { -xSize,  ySize,  zSize}, {0, 0} };
		vertices[28] = { {  xSize,  ySize, -zSize}, {1, 1} };
		vertices[29] = { { -xSize,  ySize, -zSize}, {0, 1} };
		vertices[30] = { { -xSize, -ySize, -zSize}, {0, 0} };
		vertices[31] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[32] = { { -xSize, -ySize,  zSize}, {0, 1} };
		vertices[33] = { { -xSize, -ySize,  zSize}, {0, 1} };
		vertices[34] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[35] = { {  xSize, -ySize,  zSize}, {1, 1} };
	}

	void makeCube(float xSize, float ySize, float zSize, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices)
	{
		xSize *= 0.5f;
		ySize *= 0.5f;
		zSize *= 0.5f;

		vertices.resize(24);
		vertices[0] = { { -xSize, -ySize,  zSize}, {0, 0} };
		vertices[1] = { { -xSize, -ySize,  zSize}, {1, 0} };
		vertices[2] = { { -xSize, -ySize,  zSize}, {0, 1} };
		vertices[3] = { {  xSize, -ySize,  zSize}, {1, 0} };
		vertices[4] = { {  xSize, -ySize,  zSize}, {0, 0} };
		vertices[5] = { {  xSize, -ySize,  zSize}, {1, 1} };
		vertices[6] = { {  xSize,  ySize,  zSize}, {1, 1} };
		vertices[7] = { {  xSize,  ySize,  zSize}, {0, 1} };
		vertices[8] = { {  xSize,  ySize,  zSize}, {1, 0} };
		vertices[9] = { { -xSize,  ySize,  zSize}, {0, 1} };
		vertices[10] = { { -xSize,  ySize,  zSize}, {1, 1} };
		vertices[11] = { { -xSize,  ySize,  zSize}, {0, 0} };
		vertices[12] = { { -xSize, -ySize, -zSize}, {0, 0} };
		vertices[13] = { { -xSize, -ySize, -zSize}, {0, 0} };
		vertices[14] = { { -xSize, -ySize, -zSize}, {1, 0} };
		vertices[15] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[16] = { {  xSize, -ySize, -zSize}, {1, 0} };
		vertices[17] = { {  xSize, -ySize, -zSize}, {0, 0} };
		vertices[18] = { {  xSize,  ySize, -zSize}, {0, 1} };
		vertices[19] = { {  xSize,  ySize, -zSize}, {1, 1} };
		vertices[20] = { {  xSize,  ySize, -zSize}, {1, 1} };
		vertices[21] = { { -xSize,  ySize, -zSize}, {0, 1} };
		vertices[22] = { { -xSize,  ySize, -zSize}, {0, 1} };
		vertices[23] = { { -xSize,  ySize, -zSize}, {1, 1} };
		const static uint32_t idxs[36] = {
			//front
				0,  3,  6,  0,  6,  9,
				//back
				14, 18, 17, 14, 23, 18,
				// left
				13,  1, 22,  1, 10, 22,
				// right
				16,  7,  4, 16, 19,  7,
				// top
				11,  8, 20, 11, 20, 21,
				// bottom
				12, 15,  2,  2, 15,  5
		};

		indices.resize(36);
		memcpy(indices.data(), idxs, sizeof(idxs));
	}

	void makeCylinder(float height, float radius, unsigned int nSegments, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices)
	{
		height *= 0.5f;
		const float angle = (PI_F * 2.0f) / (float)nSegments;
		vertices.resize(2 + 4 * (nSegments + 1));
		indices.resize(12 * nSegments);

		vertices[4 * (nSegments + 1)] = { { 0,  height, 0 }, {0.5f, 0.5f} };
		vertices[4 * (nSegments + 1) + 1] = { { 0, -height, 0 },  {0.5f, 0.5f} };

		for (unsigned int i = 0; i < nSegments; ++i)
		{
			const Vec3 normal(cos(i * angle), 0, sin(i * angle));
			const float normalizedX = (float)i / (float)nSegments;
			vertices[4 * i] = { { normal.x * radius, height, normal.z * radius}, {normalizedX, 1} };
			vertices[4 * i + 1] = { { normal.x * radius, -height, normal.z * radius }, { normalizedX, 0 } };
			vertices[4 * i + 2] = { { normal.x * radius, height, normal.z * radius}, Vec2(normal.x, normal.z) * 0.5f + 0.5f };
			vertices[4 * i + 3] = { { normal.x * radius, -height, normal.z * radius }, Vec2((normal.x), normal.z) * 0.5f + 0.5f };

			// todo: split vertices at edge of the caps to have hard normal transitions
			indices[12 * i] = 4 * (i + 1) + 2; // upper cap
			indices[12 * i + 1] = 4 * i + 2;
			indices[12 * i + 2] = 4 * (nSegments + 1);

			indices[12 * i + 3] = 4 * i + 3;
			indices[12 * i + 4] = 4 * (i + 1) + 3;
			indices[12 * i + 5] = 4 * (nSegments + 1) + 1;


			indices[12 * i + 6] = 4 * (i + 1) + 1;
			indices[12 * i + 7] = 4 * i + 1;
			indices[12 * i + 8] = 4 * (i + 1);

			indices[12 * i + 9] = 4 * i + 1;
			indices[12 * i + 10] = 4 * i;
			indices[12 * i + 11] = 4 * (i + 1);
		}

		vertices[4 * nSegments] = { Vec3(1, height, 0) * radius, {1, 1} };
		vertices[4 * nSegments + 1] = { Vec3(1, -height, 0) * radius, {1, 0} };
		vertices[4 * nSegments + 2] = { Vec3(1, height, 0) * radius, {1, 0.5} };
		vertices[4 * nSegments + 3] = { Vec3(1, -height, 0) * radius, {1, 0.5} };
	}

	void makeSphere(float radius, int nSegmentsLatitude, int nSegmentsLongitude, std::vector<VertexData>& vertices, std::vector<uint32_t>& indices)
	{
		BRWL_EXCEPTION(nSegmentsLatitude > 1, nullptr);
		BRWL_EXCEPTION(nSegmentsLongitude > 2, nullptr);

		const float lonStep = (2.0f * PI_F) / (float)nSegmentsLongitude;
		const float latStep = PI_F / (float)nSegmentsLatitude;

		vertices.reserve((nSegmentsLongitude + 1) * (nSegmentsLatitude + 1));
		indices.reserve(6 * (nSegmentsLongitude + 2) * (nSegmentsLatitude + 1));
		for (int lat = 0; lat <= nSegmentsLatitude; ++lat)
		{
			const float latAngle = (float)lat * latStep - PI_F * 0.5f;
			const float normalizedLat = (float)lat / (float)nSegmentsLatitude;
			const float localRadius = cos(latAngle) * radius;
			const float y = sin(latAngle) * radius;
			for (int lon = 0; lon <= nSegmentsLongitude; ++lon)
			{
				const Vec3 point({ cos((float)lon * lonStep) * localRadius, y, sin((float)lon * lonStep) * localRadius });
				vertices.push_back({ point, {(float)lon / (float)nSegmentsLongitude, normalizedLat} });
			}
		}

		for (int lat = 0; lat < nSegmentsLatitude; ++lat)
		{
			for (int lon = 0; lon < nSegmentsLongitude; ++lon)
			{
				indices.emplace_back(lat * (nSegmentsLongitude + 1) + lon + 1);
				indices.emplace_back(lat * (nSegmentsLongitude + 1) + lon);
				indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1) + lon + 1);
				indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1) + lon + 1);
				indices.emplace_back(lat * (nSegmentsLongitude + 1) + lon);
				indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1) + lon);
			}
		}

		//// fix seam
		//for (int lat = 0; lat < nSegmentsLatitude; ++lat)
		//{
		//	indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1)) + 2;
		//	indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1)) + 1;
		//	indices.emplace_back((lat + 2) * (nSegmentsLongitude + 1) + 2);
		//	indices.emplace_back((lat + 2) * (nSegmentsLongitude + 1) + 2);
		//	indices.emplace_back((lat + 1) * (nSegmentsLongitude + 1) + 1);
		//	indices.emplace_back((lat + 2) * (nSegmentsLongitude + 1) + 1);
		//}

	}


} // namespace ProceduralGeometry

BRWL_NS_END