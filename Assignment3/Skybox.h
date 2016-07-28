#include <vector>
#include <glm\glm.hpp>
//#include <stb_image.h>

#include "opengl.h"

class Skybox
{
private:
	const GLuint DEFAULT_TEXTURE_CHANNEL = GL_TEXTURE1;

	GLuint VAO;
	GLuint positionVBO;
	GLuint texture;	
	GLuint textureChannel;

	int positionAttr;

	std::vector<float> vertices;
	std::vector<std::string> faceTexturePaths;
		
	void loadVBOs();
	void loadVAO();
	void loadTextures();
	void disableAttributes();

public:	
	Skybox();
	Skybox(GLuint textureChannel);
	void load();	
	void attachToVAO(int positionAttr);
	void generateTextureUniform(GLuint* shaderId, std::string uniformName);
	void draw();
};

Skybox::Skybox(GLuint textureChannel)
{
	this->textureChannel = textureChannel;
}

Skybox::Skybox()
{
	textureChannel = DEFAULT_TEXTURE_CHANNEL;
}

void Skybox::load()
{
	vertices = std::vector<float>
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// scale vertices
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i] *= 100.0;
	}

	std::string basePath = "C:\\Users\\Evan\\Documents\\Visual Studio 2015\\Projects\\RealTimeExamples\\Assignment3\\tiles\\";
	faceTexturePaths = {
		basePath + "skybox-front.png",
		basePath + "skybox-left.png",
		basePath + "skybox-right.png",
		basePath + "skybox-top.png",
		basePath + "skybox-bottom.png",
		basePath + "skybox-back.png",
	};

	loadVBOs();	
	loadVAO();
	loadTextures();

	// check for errors
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
}

void Skybox::attachToVAO(int positionAttr)
{
	// store attribute reference
	this->positionAttr = positionAttr;

	if (positionVBO != 0)
	{
		// bind VAO and attach position VBO to VAO
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
		glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Enable the attribute 
		glEnableVertexAttribArray(positionAttr);

		// unbind the VAO
		glBindVertexArray(0);
	}
}

void Skybox::generateTextureUniform(GLuint* shaderId, std::string uniformName)
{
	// generate uniform reference and pass it to shader
	GLuint iSkyBoxCubeSamplerLoc = glGetUniformLocation(*shaderId, uniformName.c_str());
	if (iSkyBoxCubeSamplerLoc != -1)
	{
		glActiveTexture(textureChannel);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		glUniform1i(iSkyBoxCubeSamplerLoc, textureChannel - GL_TEXTURE0);
	}
}

void Skybox::draw()
{
	// disable depth mask
	glDepthMask(GL_FALSE);

	// Can now bind the vertex array object to the graphics pipeline, to render with it
	glBindVertexArray(VAO);

	// draw the vertices as triangles
	glDrawArrays(
		GL_TRIANGLES,	// mode
		0,				// first index
		vertices.size()	// count
	);

	// unbind VAO
	glBindVertexArray(0);

	// disable the attribute after drawing
	//disableAttributes();

	// enable depth mask
	glDepthMask(GL_TRUE);
}

void Skybox::loadVBOs()
{
	// load vertices into position VBO
	positionVBO = 0;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* vertices.size(), &vertices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Skybox::loadVAO()
{
	VAO = 0;
	glGenVertexArrays(1, &VAO);
}

void Skybox::loadTextures()
{
	// generate textureID and activate texture channel
	texture = 0;
	glGenTextures(1, &texture);
	glActiveTexture(textureChannel);

	// bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	// check for errors
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}

	// load each face texture from image files and upload them to opengl
	for (int i = 0; i < faceTexturePaths.size(); i++)
	{
		int imgWidth, imgHeight;
		int nColorDepth;
		std::string texturePath = faceTexturePaths.at(i);

		// load pixel data
		unsigned char* pixels = stbi_load(texturePath.c_str(), &imgWidth, &imgHeight, &nColorDepth, 0);

		if (pixels == NULL)
		{
			fprintf(stderr, "Failed to load texture file: %s\n", texturePath.c_str());
			continue;
		}

		// upload to opengl as 2D texture image
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGBA8,
			imgWidth,
			imgHeight,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels
		);		

		// check for errors
		GLenum err1;
		while ((err1 = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err1 << std::endl;
		}

		// set filtering and wrapping parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);		
	}

	// unbind texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Skybox::disableAttributes()
{
	glDisableVertexAttribArray(positionAttr);
}

