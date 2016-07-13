#include <vector>
#include "PerlinNoise.h"

class TerrainMesh
{
private:
	const int MAX_TERRAIN_X = 100.0f;
	const int MAX_TERRAIN_Z = 100.0f;
	const int MAX_TERRAIN_HEIGHT = 100;
	int TERRAIN_X, TERRAIN_Z;
	float Y_POSITION;	
	PerlinNoise perlinNoise;
	float Basis(glm::vec3);
	float generateHeight(glm::vec3 point, float H, float lacunarity, int octaves);

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
	perlinNoise = PerlinNoise(7);
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
}

void TerrainMesh::generate()
{
	float fTerrainX, fTerrainZ;

	// generate vertices
	fTerrainZ = -MAX_TERRAIN_Z / 2;
	for (int z = 0; z < TERRAIN_Z; z++)
	{
		fTerrainX = -MAX_TERRAIN_X / 2;
		for (int x = 0; x < TERRAIN_X; x++)
		{
			// generate height for the vertex
			float height = generateHeight(glm::vec3(fTerrainX, Y_POSITION, fTerrainZ), 0, 2.0, 7);

			// store vertex
			vertices.push_back(fTerrainX);
			vertices.push_back(height);
			vertices.push_back(fTerrainZ);

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
float TerrainMesh::Basis(glm::vec3 point)
{
	//float r = rand() / (float)RAND_MAX;
	//return r * 3.0f;
	
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
float TerrainMesh::generateHeight(glm::vec3 point, float H, float lacunarity, int octaves)
{
	float value, frequency, remainder;
	int i;
	bool first = true;
	const int MAX_OCTAVES = 5;
	float exponent_array[MAX_OCTAVES];

	/* precompute and store spectral weights */
	if (first) 
	{
		frequency = 1.0;
		for (i = 0; i<MAX_OCTAVES; i++)
		{
			/* compute weight for each frequency */
			exponent_array[i] = pow(frequency, -H);
			frequency *= lacunarity;
		}
		first = false;
	}

	value = 0.0;
	/* inner loop of spectral construction */
	for (i = 0; i<octaves; i++) 
	{
		float noise = Basis(point);
		value += noise;// *exponent_array[i];
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	} 
	
	remainder = octaves - (int)octaves;
	if (remainder) /* add in "octaves" remainder */
	{
		/* "i" and spatial freq. are preset in loop above */
		value += remainder * Basis(point) * exponent_array[i];
	}
	return(value);
}

