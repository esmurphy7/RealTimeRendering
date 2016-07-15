#include <vector>
#include "PerlinNoise.h"
#include "PPM_File.h"
#include "HeightMap.h"

class TerrainMesh
{
private:
	const int MAX_TERRAIN_X = 100.0f;
	const int MAX_TERRAIN_Z = 100.0f;
	const int MAX_TERRAIN_HEIGHT = 100;	
	float Y_POSITION;	
	PerlinNoise perlinNoise;

	float generatePerlinNoise(glm::vec3);
	float fBm(glm::vec3 point, float H, float lacunarity, int octaves);

public:
	int TERRAIN_X, TERRAIN_Z;
	HeightMap heightMap;

	std::vector<float>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<float>			normals;
	std::vector<float>			textureCoords;

	TerrainMesh(int, int, float);
	void generate();
};

TerrainMesh::TerrainMesh(int terrainX, int terrainZ, float yPos)
{
	TERRAIN_X = terrainX;
	TERRAIN_Z = terrainZ;
	Y_POSITION = yPos;
	perlinNoise = PerlinNoise(7);
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
	normals = std::vector<float>();
	textureCoords = std::vector<float>();
}

void TerrainMesh::generate()
{
	float fTerrainX, fTerrainZ;

	// 1st pass: generate heightmap
	std::vector<float> heights = std::vector<float>();
	float largestHeight = 0;
	fTerrainZ = -MAX_TERRAIN_Z / 2;
	for (int z = 0; z < TERRAIN_Z; z++)
	{
		fTerrainX = -MAX_TERRAIN_X / 2;
		for (int x = 0; x < TERRAIN_X; x++)
		{
			//float height = fBm(glm::vec3(fTerrainX, Y_POSITION, fTerrainZ), 0, 1.0, 7);
			float height = generatePerlinNoise(glm::vec3(fTerrainX, Y_POSITION, fTerrainZ));
			if (height > largestHeight)
			{
				largestHeight = height;
			}
			heights.push_back(height);

			fTerrainX += (MAX_TERRAIN_X / (TERRAIN_X - 1));
		}
		fTerrainZ += (MAX_TERRAIN_Z / (TERRAIN_Z - 1));
	}
	heightMap = HeightMap(TERRAIN_X, TERRAIN_Z, heights);	
	heightMap.saveToPPMFile("heightmap.ppm");

	// 2nd pass: generate vertices, indices, normals, and texture coordinates
	fTerrainX, fTerrainZ = 0;
	fTerrainZ = -MAX_TERRAIN_Z / 2;
	for (int z = 0; z < TERRAIN_Z; z++)
	{
		fTerrainX = -MAX_TERRAIN_X / 2;
		for (int x = 0; x < TERRAIN_X; x++)
		{			
			// store texture coords
			float texU = (float)x / (float)TERRAIN_X;
			float texV = (float)z / (float)TERRAIN_Z;
			textureCoords.push_back(texU);
			textureCoords.push_back(texV);

			// store vertex
			glm::vec3 vertex = glm::vec3(fTerrainX, Y_POSITION, fTerrainZ);
			vertices.push_back(vertex.x);
			vertices.push_back(vertex.y);
			vertices.push_back(vertex.z);

			fTerrainX += (MAX_TERRAIN_X / (TERRAIN_X - 1));
		}
		fTerrainZ += (MAX_TERRAIN_Z / (TERRAIN_Z - 1));
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

/*
*  Perlin noise basis function
*/
float TerrainMesh::generatePerlinNoise(glm::vec3 point)
{
	float r = perlinNoise.noise(point.x, point.z, 0.8);
	return r;
}

/*
* Procedural fBm evaluated at "point"; returns value stored in "value".
*
* Parameters:
* "H" is the fractal increment
* "lacunarity" is the gap between successive frequencies
* "octaves" is the number of frequencies in the fBm
*/
float TerrainMesh::fBm(glm::vec3 point, float H, float lacunarity, int octaves)
{
	float value, frequency, remainder;
	int i;
	bool first = true;
	const int MAX_OCTAVES = 10;
	std::vector<float> exponent_array = std::vector<float>();

	/* precompute and store spectral weights */
	if (first) 
	{
		frequency = 1.0;
		for (i = 0; i<MAX_OCTAVES; i++)
		{
			/* compute weight for each frequency */
			exponent_array.push_back(pow(frequency, -H));
			frequency *= lacunarity;
		}
		first = false;
	}

	value = 0.0;
	/* inner loop of spectral construction */
	for (i = 0; i<octaves; i++) 
	{
		float noise = generatePerlinNoise(point);
		value += noise *exponent_array.at(i);
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} 
	
	remainder = octaves - (int)octaves;
	if (remainder) /* add in "octaves" remainder */
	{
		/* "i" and spatial freq. are preset in loop above */
		value += remainder * generatePerlinNoise(point) * exponent_array[i];
	}
	return(value);
}

