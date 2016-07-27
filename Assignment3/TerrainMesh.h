#include <vector>
#include "PerlinNoise.h"
#include "SimplexNoise.h"
#include "PPM_File.h"
#include "HeightMap.h"

class TerrainMesh
{
private:
	const GLuint DEFAULT_TEXTURE_CHANNEL = GL_TEXTURE0;

	float Y_POSITION;

	GLuint VAO;
	GLuint positionVBO;
	GLuint texcoordVBO;
	GLuint normalVBO;
	GLuint heightMapVBO;
	GLuint indicesEBO;
	GLuint textureArrayId;	
	GLuint textureChannel;

	int positionAttr, texCoordAttr, normalsAttr, heightMapAttr;

	void generate();
	void loadVBOs();
	void loadEBOs();
	void loadVAO();
	void loadTextures();
	void disableAttributes();

public:
	int TERRAIN_X, TERRAIN_Z;
	HeightMap heightMap;

	std::vector<float>			vertices = std::vector<float>();
	std::vector<unsigned int>	indices = std::vector<unsigned int>();
	std::vector<float>			normals = std::vector<float>();
	std::vector<float>			textureCoords = std::vector<float>();

	TerrainMesh(int, int, float);
	TerrainMesh(int, int, float, GLuint textureChannel);
	void load();	
	void attachToVAO(int positionAttr, int texCoordAttr, int normalsAttr, int heightMapAttr);
	void generateTextureUniform(GLuint* shaderId, std::string uniformName);
	void draw();
};

TerrainMesh::TerrainMesh(int width, int height, float yPos)
{
	TERRAIN_X = width;
	TERRAIN_Z = height;
	Y_POSITION = yPos;
	textureChannel = DEFAULT_TEXTURE_CHANNEL;
}

TerrainMesh::TerrainMesh(int width, int height, float yPos, GLuint textureChannel)
{
	TERRAIN_X = width;
	TERRAIN_Z = height;
	Y_POSITION = yPos;
	this->textureChannel = textureChannel;
}

void TerrainMesh::load()
{
	// generate heightmap,  vertices, texturecoords, normals, and indices
	generate();

	// load VBOs
	loadVBOs();

	// load EBOs
	loadEBOs();

	// load VAO
	loadVAO();

	// load textures
	loadTextures();

	// check for errors
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
}

void TerrainMesh::attachToVAO(int positionAttr, int texCoordAttr, int normalsAttr, int heightMapAttr)
{
	// store each attribute value in the class
	this->positionAttr = positionAttr;
	this->texCoordAttr = texCoordAttr;
	this->normalsAttr = normalsAttr;
	this->heightMapAttr = heightMapAttr;

	// Attach position buffer as attribute 0
	if (positionVBO != 0)
	{
		glBindVertexArray(VAO);

		// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
		// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
		glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
		glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
		glEnableVertexAttribArray(positionAttr);

		glBindVertexArray(0);
	}

	// Attach texcoord buffer as attribute 1
	if (texcoordVBO != 0)
	{
		glBindVertexArray(VAO);

		// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
		// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
		glEnableVertexAttribArray(texcoordVBO);

		glBindVertexArray(0);
	}

	// Attach normal buffer as attribute 2
	if (normalVBO != 0)
	{
		glBindVertexArray(VAO);

		// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
		// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glVertexAttribPointer(normalsAttr, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
		glEnableVertexAttribArray(normalsAttr);

		glBindVertexArray(0);
	}

	// attach the heightmap VBO to the mesh VAO
	if (heightMapVBO != 0)
	{
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, heightMapVBO);
		glVertexAttribPointer(heightMapAttr, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(heightMapAttr);
		glBindVertexArray(0);
	}

	// attach the index EBO to the mesh VAO
	if (indicesEBO != 0)
	{
		glBindVertexArray(VAO);

		// Note: Calling glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); when a VAO is bound attaches the index buffer to the VAO.
		// From an API design perspective, this is subtle.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);

		glBindVertexArray(0);
	}	
}

void TerrainMesh::generateTextureUniform(GLuint* shaderId, std::string uniformName)
{
	// generate uniform reference and pass it to shader
	GLuint iTextureArraySamplerLoc = glGetUniformLocation(*shaderId, uniformName.c_str());
	if (iTextureArraySamplerLoc != -1)
	{
		glActiveTexture(textureChannel);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);
		glUniform1i(iTextureArraySamplerLoc, textureChannel);
	}
}

void TerrainMesh::draw()
{
	// Can now bind the vertex array object to the graphics pipeline, to render with it
	glBindVertexArray(VAO);

	// Draw the vertices		
	glDrawElements(
		GL_TRIANGLES,		// mode
		indices.size(),		// count
		GL_UNSIGNED_INT,	// type
		(void*)0			// element array buffer offset
	);

	// unbind VAO
	glBindVertexArray(0);

	// disable the attribute after drawing
	//disableAttributes();
}

void TerrainMesh::generate()
{	
	// generate heightmap
	heightMap = HeightMap(TERRAIN_X, TERRAIN_Z, 1);	
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

			// store vertex
			//glm::vec3 vertex = glm::vec3(float(x), heightMap.getHeightAt(float(x), float(z)), float(z));
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

void TerrainMesh::loadVBOs()
{
	// initialize vertex VBO
	positionVBO = 0;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* vertices.size(), &vertices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// initialize UVcoords VBO	
	texcoordVBO = 0;
	if (!textureCoords.empty())
	{
		glGenBuffers(1, &texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * textureCoords.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * textureCoords.size(), &textureCoords[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// initialize normals VBO	
	normalVBO = 0;
	if (!normals.empty())
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * normals.size(), &normals[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// initialize heightmap VBO
	heightMapVBO = 0;
	std::vector<float> heights = heightMap.getAsFloatVector(GL_RED);
	if (!heights.empty())
	{
		glGenBuffers(1, &heightMapVBO);
		glBindBuffer(GL_ARRAY_BUFFER, heightMapVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * heights.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * heights.size(), &heights[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void TerrainMesh::loadEBOs()
{
	// initialize indices EBO
	indicesEBO = 0;
	glGenBuffers(1, &indicesEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * indices.size(), &indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TerrainMesh::loadVAO()
{
	VAO = 0;
	glGenVertexArrays(1, &VAO);
}

void TerrainMesh::loadTextures()
{
	const unsigned int TextureWidth = 512;
	const unsigned int TextureHeight = 512;

	// initialize set of texture paths
	std::vector<std::string> texturePaths = {
		"C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\snow.tga",
		"C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\rock.tga",
		"C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\sand.tga",
		"C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\grass.tga",
		"C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\water.tga",
	};

	// generate and bind texture object
	textureArrayId = 0;
	glGenTextures(1, &textureArrayId);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);

	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,		// target
		0,							// level			
		GL_RGBA8,					// internal format		
		TextureWidth,				// width
		TextureHeight,				// height
		texturePaths.size(),		// depth
		0,							// border
		GL_RGBA,					// format
		GL_UNSIGNED_BYTE,			// type
		NULL						// data
	);

	// load each texture path into the opengl array texture
	int layerNumber = 0;
	for (int i = 0; i < texturePaths.size(); i++)
	{
		std::string texturePath = texturePaths.at(i);

		int imgWidth;
		int imgHeight;
		int nColorDepth;

		// load the texture file
		unsigned char* pixels = stbi_load(texturePath.c_str(), &imgWidth, &imgHeight, &nColorDepth, 0);
		if (pixels == NULL)
		{
			fprintf(stderr, "Failed to load texture file: %s\n", texturePath.c_str());
			continue;
		}
		if (imgWidth != TextureWidth || imgHeight != TextureHeight)
		{
			fprintf(stderr, "Image dimensions does not match texture dimensions: %s\n", texturePath.c_str());
			continue;
		}

		// upload texture data
		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,	// target
			0,						// level
			0,						// xoffset
			0,						// yoffset 
			layerNumber,			// zoffset
			imgWidth,				// width
			imgHeight,				// height
			1, 						// depth
			GL_RGBA,				// format
			GL_UNSIGNED_BYTE,		// type
			pixels					// data	
		);

		// check for errors
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}

		// set filtering parameters
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// target next texture
		layerNumber++;
	}

	// unbind texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TerrainMesh::disableAttributes()
{
	glDisableVertexAttribArray(positionAttr);
	glDisableVertexAttribArray(texCoordAttr);
	glDisableVertexAttribArray(normalsAttr);
	glDisableVertexAttribArray(heightMapAttr);
}