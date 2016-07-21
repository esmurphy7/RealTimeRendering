#pragma once
#include <vector>
#include <iostream>
#include "PPM_File.h"
#include "SimplexNoise.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class HeightMap
{
private:
	std::vector<glm::vec3>	rgbData = std::vector<glm::vec3>();
	std::map<GLint, std::vector<GLint>> supportedFormatsByDataType;
	PerlinNoise perlinNoise;
	glm::vec3			baseColor = glm::vec3(1.0, 0.0, 0.0);
	float largestHeight = 0.0;

	bool supportedFormat(GLint dataType, GLint dataFormat);
	void generate();	

public:
	unsigned int WIDTH, HEIGHT;	

	HeightMap();
	HeightMap(unsigned int width, unsigned int height, unsigned int noiseSeed);

	float getHeightAt(glm::vec2 point);
	float getHeightAt(int x, int y);
	void setBaseColor(glm::vec3 baseColor);		
	std::vector<float> getAsFloatVector(GLint format);
	std::vector<unsigned char> getAsByteVector(GLint format);
	void saveToPPMFile(std::string fileName);
	void saveToPNGFile(std::string filename);
};

HeightMap::HeightMap()
{

}

HeightMap::HeightMap(unsigned int width, unsigned int height, unsigned int seed)
{
	WIDTH = width;
	HEIGHT = height;
	perlinNoise = PerlinNoise(seed);
	supportedFormatsByDataType = std::map<GLint, std::vector<GLint>> {
		{GL_FLOAT, std::vector<GLint> {GL_RED, GL_RGB, GL_RGBA}},
		{GL_UNSIGNED_BYTE, std::vector<GLint> {GL_RED, GL_RGB, GL_RGBA}}
	};
	generate();
}

void HeightMap::generate()
{	
	SimplexNoise simplexNoise = SimplexNoise(1.0, 100.0);
	for (int z = 0; z < HEIGHT; z++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			// generate color and store as texture data
			//float R = simplexNoise.fractal(3, float(x), float(z));
			float R = perlinNoise.noise(float(x), float(z), 0.8);
			rgbData.push_back(glm::vec3(R, baseColor.g, baseColor.b));
		}
	}
}

float HeightMap::getHeightAt(glm::vec2 point)
{
	return getHeightAt(point.x, point.y);
}

float HeightMap::getHeightAt(int x, int y)
{
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= WIDTH) x = WIDTH - 1;
	if (y >= HEIGHT) y = HEIGHT - 1;

	float height = rgbData.at(y * HEIGHT + x).r;
	return height;
}

void HeightMap::setBaseColor(glm::vec3 color)
{
	baseColor = color;
}

bool HeightMap::supportedFormat(GLint dataType, GLint dataFormat)
{
	// get list of valid formats, given the data type
	std::vector<GLint> supportedFormats = supportedFormatsByDataType.at(dataType);

	// check that the format is in list of supported formats
	if (std::find(supportedFormats.begin(), supportedFormats.end(), dataFormat) != supportedFormats.end())
	{
		return true;
	}

	return false;
}

std::vector<float> HeightMap::getAsFloatVector(GLint format)
{
	// check that format is supported
	if (!supportedFormat(GL_FLOAT, format))
	{
		std::cout << "Cannot get heightmap as given format" << std::endl;
		return std::vector<float>();
	
	}

	// convert rgb data to float vector
	std::vector<float> rgbFloats = std::vector<float>();
	for (int i = 0; i < rgbData.size(); i++)
	{
		glm::vec3 color = rgbData.at(i);
		rgbFloats.push_back(color.r);

		// make sure that the red channel wasn't specifically requested
		if (format != GL_RED)
		{
			rgbFloats.push_back(color.g);
			rgbFloats.push_back(color.b);
		}

		// check if alpa format requested
		if (format == GL_RGBA)
		{
			float alpha = 1.0;
			rgbFloats.push_back(alpha);
		}
	}
	return rgbFloats;
}

std::vector<unsigned char> HeightMap::getAsByteVector(GLint format)
{
	// check that format is supported
	if (!supportedFormat(GL_UNSIGNED_BYTE, format))
	{
		std::cout << "Cannot get heightmap as given format" << std::endl;
		return std::vector<unsigned char>();

	}

	// convert rgb data to byte vector
	std::vector<unsigned char> rgbBytes = std::vector<unsigned char>();
	for (int i = 0; i < rgbData.size(); i++)
	{
		glm::vec3 color = rgbData.at(i);
		int rByte = int(255*color.r);		
		rgbBytes.push_back(reinterpret_cast<unsigned char>(&rByte));
		
		// make sure that the red channel wasn't specifically requested
		if (format != GL_RED)
		{
			int gByte = int(255 * color.g);
			int bByte = int(255 * color.b);
			rgbBytes.push_back(reinterpret_cast<unsigned char>(&gByte));
			rgbBytes.push_back(reinterpret_cast<unsigned char>(&bByte));
		}

		// check if alpha format requested
		if (format == GL_RGBA)
		{
			int aByte = 255;
			rgbBytes.push_back(reinterpret_cast<unsigned char>(&aByte));
		}		
	}
	return rgbBytes;
}

void HeightMap::saveToPPMFile(std::string fileName)
{
	PPM_File ppmFile;
	ppmFile.open(fileName, WIDTH, HEIGHT);
	for (int i = 0; i < rgbData.size(); i += 3)
	{
		glm::vec3 color;
		color = rgbData.at(i);
		ppmFile.writeColor(color);
	}
	ppmFile.close();
}

void HeightMap::saveToPNGFile(std::string filename)
{
	// write the pixel data to png file
	int colorDepth = 3;
	bool useRGBA = false;
	int stride = WIDTH*colorDepth;
	if (stbi_write_png(filename.c_str(), WIDTH, HEIGHT, colorDepth, getAsByteVector(GL_RGBA).data(), stride) == 0)
	{
		std::cout << "Failed to write to: " << filename.c_str() << std::endl;
		return;
	}
}