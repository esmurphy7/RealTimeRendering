#include <vector>

class TerrainMesh
{
private:
	int TERRAIN_X, TERRAIN_Z;
	float Y_POSITION;	
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
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
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
			// generate height for the vertex
			float height = generateHeight(glm::vec3(fTerrainX, Y_POSITION, fTerrainZ), 0, 2.0, 7);

			// store vertex
			vertices.push_back(fTerrainX);
			vertices.push_back(height);
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

/*
*  Perlin noise basis function
*/
float Basis(glm::vec3 point)
{
	float r = rand() / (float)RAND_MAX;
	return r * 3.0f;
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

