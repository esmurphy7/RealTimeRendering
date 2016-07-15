#pragma once
#include <vector>
#include <iostream>
#include "PPM_File.h"

class HeightMap
{
private:
	glm::vec3			baseColor = glm::vec3(1.0, 0.0, 0.0);
	std::vector<float>	heights = std::vector<float>();
	float				largestHeight = 0.0;

	void findLargestHeight();
	void generatePixelData();

public:
	unsigned int		WIDTH, HEIGHT;
	std::vector<float>	pixelData = std::vector<float>();

	HeightMap();
	HeightMap(unsigned int width, unsigned int height, std::vector<float> heights);

	float getHeightAt(glm::vec2 point);
	float getHeightAt(int x, int y);
	void setBaseColor(glm::vec3 baseColor);
	void saveToPPMFile(std::string fileName);
};

HeightMap::HeightMap()
{

}

HeightMap::HeightMap(unsigned int width, unsigned int height, std::vector<float> heights)
{
	WIDTH = width;
	HEIGHT = height;
	this->heights = heights;
	findLargestHeight();
	generatePixelData();
}

void HeightMap::findLargestHeight()
{
	for (int i = 0; i < heights.size(); i++)
	{
		if (heights.at(i) > largestHeight)
		{
			largestHeight = heights.at(i);
		}
	}
}

void HeightMap::generatePixelData()
{
	for (int i = 0; i < heights.size(); i++)
	{
		// generate color and store as texture data
		float R = baseColor.r - (heights.at(i) / largestHeight);
		pixelData.push_back(R);
		pixelData.push_back(baseColor.g);
		pixelData.push_back(baseColor.b);
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

	float height = heights.at(y * HEIGHT + x);
	return height;
}

void HeightMap::setBaseColor(glm::vec3 color)
{
	baseColor = color;
}

void HeightMap::saveToPPMFile(std::string fileName)
{
	// ensure pixelData is in RGB format
	if (pixelData.size() % 3 != 0)
	{
		std::cout << "Cannot write to ppm file: pixelData is not multiple of three";
		return;
	}

	PPM_File ppmFile;
	ppmFile.open(fileName, WIDTH, HEIGHT);
	for (int i = 0; i < pixelData.size(); i += 3)
	{
		glm::vec3 color;
		color.r = pixelData.at(i);
		color.g = pixelData.at(i + 1);
		color.b = pixelData.at(i + 2);
		ppmFile.writeColor(color);
	}
	ppmFile.close();
}