#include <vector>

class TerrainMesh
{
private:
	int TERRAIN_X, TERRAIN_Z;
	float Y_POSITION;	

public:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> heights;

	TerrainMesh(int, int, float);
	void generate();
};

TerrainMesh::TerrainMesh(int terrainX, int terrainZ, float yPos)
{
	TERRAIN_X = terrainX;
	TERRAIN_Z = terrainZ;
	Y_POSITION = yPos;
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
	heights = std::vector<float>();
}

void TerrainMesh::generate()
{
	const int TERRAIN_VERTEXCOUNT = (TERRAIN_X * TERRAIN_Z);

	// The triangle count is the number of quads (which is one
	// less than each dimension of the terrain vertex map) multiplied
	// by two (as we need two triangles to make up each quad).
	const int TERRAIN_TRIANGLECOUNT = ((TERRAIN_X - 1)*(TERRAIN_Z - 1) * 2);
	const int TERRAINSIZE = 25.0f;
	const int TERRAINHEIGHT = 10;

	float fTerrainX, fTerrainZ;

	// generate vertices
	fTerrainZ = -TERRAINSIZE / 2;
	for (int z = 0; z < TERRAIN_Z; z++)
	{
		fTerrainX = -TERRAINSIZE / 2;
		for (int x = 0; x < TERRAIN_X; x++)
		{
			vertices.push_back(fTerrainX);
			vertices.push_back(Y_POSITION);
			vertices.push_back(fTerrainZ);

			fTerrainX += (TERRAINSIZE / (TERRAIN_X - 1));
		}
		fTerrainZ += (TERRAINSIZE / (TERRAIN_Z - 1));
	}

	// generate indices
	for (int x = 0; x < TERRAIN_X - 1; x++)
	{
		for (int z = 0; z < TERRAIN_Z - 1; z++)
		{
			// build indices for first triangle in the quad
			indices.push_back(x + z * TERRAIN_X);
			indices.push_back(x + (z + 1) * TERRAIN_X);
			indices.push_back((x + 1) + z * TERRAIN_X);

			// build indices for second triangle in the quad
			indices.push_back(x + (z + 1) * TERRAIN_X);
			indices.push_back((x + 1) + (z + 1) * TERRAIN_X);
			indices.push_back((x + 1) + z * TERRAIN_X);
		}
	}
}