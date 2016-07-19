#pragma once
#include <vector>
#include <iostream>
#include "PPM_File.h"
#include "SimplexNoise.h"

class HeightMap
{
private:
	glm::vec3 baseColor = glm::vec3(1.0, 0.0, 0.0);
	float largestHeight = 0.0;

	void generate();

public:
	unsigned int WIDTH, HEIGHT;
	std::vector<glm::vec3>	rgbData = std::vector<glm::vec3>();

	HeightMap();
	HeightMap(unsigned int width, unsigned int height);

	float getHeightAt(glm::vec2 point);
	float getHeightAt(int x, int y);
	void setBaseColor(glm::vec3 baseColor);
	void saveToPPMFile(std::string fileName);
};

HeightMap::HeightMap()
{

}

HeightMap::HeightMap(unsigned int width, unsigned int height)
{
	WIDTH = width;
	HEIGHT = height;
	generate();
}

void HeightMap::generate()
{
	SimplexNoise simplexNoise = SimplexNoise(1.0, 1.0);
	for (int z = 0; z < HEIGHT; z++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			// generate color and store as texture data
			float R = simplexNoise.fractal(3, float(x), float(z));
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