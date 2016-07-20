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
	PerlinNoise perlinNoise;
	glm::vec3 baseColor = glm::vec3(1.0, 0.0, 0.0);
	float largestHeight = 0.0;

	void generate();	

public:
	unsigned int WIDTH, HEIGHT;	

	HeightMap();
	HeightMap(unsigned int width, unsigned int height, unsigned int noiseSeed);

	float getHeightAt(glm::vec2 point);
	float getHeightAt(int x, int y);
	void setBaseColor(glm::vec3 baseColor);	
	std::vector<float> getRGBDataAsFloatVector(bool asRGBA);
	std::vector<unsigned char> getRGBDataAsByteVector(bool asRGBA);
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

std::vector<float> HeightMap::getRGBDataAsFloatVector(bool asRGBA)
{
	// format the heightmap data as rgba bytes
	std::vector<float> rgbFloats = std::vector<float>();
	for (int i = 0; i < rgbData.size(); i++)
	{
		glm::vec3 color = rgbData.at(i);
		rgbFloats.push_back(color.r);
		rgbFloats.push_back(color.g);
		rgbFloats.push_back(color.b);

		if (asRGBA)
		{
			float alpha = 1.0;
			rgbFloats.push_back(alpha);
		}
	}
	return rgbFloats;
}

std::vector<unsigned char> HeightMap::getRGBDataAsByteVector(bool asRGBA)
{
	// format the heightmap data as rgba bytes
	std::vector<unsigned char> rgbBytes = std::vector<unsigned char>();
	for (int i = 0; i < rgbData.size(); i++)
	{
		glm::vec3 color = rgbData.at(i);
		int rByte = int(255*color.r);
		int gByte = int(255*color.g);
		int bByte = int(255*color.b);
		/*int rByte = int(color.r);
		int gByte = int(color.g);
		int bByte = int(color.b);*/
		rgbBytes.push_back(reinterpret_cast<unsigned char>(&rByte));
		rgbBytes.push_back(reinterpret_cast<unsigned char>(&gByte));
		rgbBytes.push_back(reinterpret_cast<unsigned char>(&bByte));

		if (asRGBA)
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
	if (stbi_write_png(filename.c_str(), WIDTH, HEIGHT, colorDepth, getRGBDataAsByteVector(useRGBA).data(), stride) == 0)
	{
		std::cout << "Failed to write to: " << filename.c_str() << std::endl;
		return;
	}
}