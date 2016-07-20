#include <vector>
#include "PerlinNoise.h"
#include "SimplexNoise.h"
#include "PPM_File.h"
#include "HeightMap.h"

class TerrainMesh
{
private:
	float Y_POSITION;

public:
	int TERRAIN_X, TERRAIN_Z;
	HeightMap heightMap;

	std::vector<float>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<float>			normals;
	std::vector<float>			textureCoords;
	std::vector<float>			heightMapCoords;

	TerrainMesh(int, int, float);
	void generate();
};

TerrainMesh::TerrainMesh(int width, int height, float yPos)
{
	TERRAIN_X = width;
	TERRAIN_Z = height;
	Y_POSITION = yPos;
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
	normals = std::vector<float>();
	textureCoords = std::vector<float>();
	heightMapCoords = std::vector<float>();
}

void TerrainMesh::generate()
{	
	// generate heightmap
	heightMap = HeightMap(TERRAIN_X, TERRAIN_Z, 7);	
	//heightMap.saveToPPMFile("heightmap.ppm");
	heightMap.saveToPNGFile("heightmap.png");

	// generate vertices, indices, normals, and texture coordinates
	for (int z = 0; z < TERRAIN_Z; z++)
	{
		for (int x = 0; x < TERRAIN_X; x++)
		{			
			// generate texture coords
			const int TILE_X = TERRAIN_X/16;
			const int TILE_Z = TERRAIN_Z/16;
			float texU = (float)x / (float)TILE_X;
			float texV = (float)z / (float)TILE_Z;
			textureCoords.push_back(texU);
			textureCoords.push_back(texV);

			// generate heightmap coords
			float hmU = (float)x / (float)TERRAIN_X;
			float hmV = (float)z / (float)TERRAIN_Z;
			heightMapCoords.push_back(hmU);
			heightMapCoords.push_back(hmV);

			// store vertex
			glm::vec3 vertex = glm::vec3(float(x), Y_POSITION, float(z));
			vertices.push_back(vertex.x);
			vertices.push_back(vertex.y);
			vertices.push_back(vertex.z);

			// generate indices
			if (x < TERRAIN_X - 1 &&
				z < TERRAIN_Z - 1)
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

			// generate normals by averaging neighboring heights
			glm::vec3 normal;			
			float offset = 1.0;

			float hL = heightMap.getHeightAt(vertex.x - offset, vertex.z);
			float hR = heightMap.getHeightAt(vertex.x + offset, vertex.z);
			float hD = heightMap.getHeightAt(vertex.x, vertex.z - offset);
			float hU = heightMap.getHeightAt(vertex.x, vertex.z + offset);	

			normal.x = hL - hR;
			normal.z = hD - hU;
			normal.y = 2.0;
			normal = normalize(normal);

			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
		}
	}	
}

