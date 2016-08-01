#pragma once
#include <vector>
#include <iostream>
#include "PPM_File.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class HeightMap
{
private:
	std::vector<glm::vec3>	rgbData = std::vector<glm::vec3>();
	std::vector<std::vector<float>> heights2D;
	std::map<GLint, std::vector<GLint>> supportedFormatsByDataType;
	PerlinNoise perlinNoise;
	glm::vec3			baseColor = glm::vec3(1.0, 0.0, 0.0);
	float largestHeight = 0.0;

	bool supportedFormat(GLint dataType, GLint dataFormat);
	void setHeightAt(glm::vec2 coords, float height);
	float getRandomInRange(float range);
	void generate();	
	void generateDiamondSquare(glm::vec2 topLeftCoords, unsigned int width, unsigned int height, float hRange, float scale);
	void generateTestHeightMap();
	double fBm(glm::vec2 point, double H, float lacunarity, float octaves);
	double HybridMultifractal(glm::vec2 point, double H, double lacunarity, double octaves, double offset);

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

	heights2D = std::vector<std::vector<float>>(WIDTH, std::vector<float>(HEIGHT, 0));

	generate();
	//generateDiamondSquare(glm::vec2(0,0), WIDTH, HEIGHT, 60, 1.0);
	//generateTestHeightMap();
}

void HeightMap::generateDiamondSquare(glm::vec2 topLeftCoords, unsigned int width, unsigned int height, float hRange, float scale)
{
	// base case: unit square passed in
	if (width == 1 || height == 1)
	{
		return;
	}

	// avoid off-by-one errors by decrementing dimensions
	width--;
	height--;

	// set corners' height
	int cornerHeight = getRandomInRange(hRange);

	glm::vec2 topLeft = topLeftCoords;
	glm::vec2 topRight = glm::vec2(topLeftCoords.x + width, topLeftCoords.y);
	glm::vec2 bottomLeft = glm::vec2(topLeftCoords.x, topLeftCoords.y + height);
	glm::vec2 bottomRight = glm::vec2(topLeftCoords.x + width, topLeftCoords.y + height);

	setHeightAt(topLeft, getRandomInRange(hRange) * scale);
	setHeightAt(topRight, getRandomInRange(hRange) * scale);
	setHeightAt(bottomLeft, getRandomInRange(hRange) * scale);
	setHeightAt(bottomRight, getRandomInRange(hRange) * scale);

	// (Diamond step)
	// find centre of grid
	glm::vec2 squareCenter = glm::vec2(topLeftCoords.x + width / 2, topLeftCoords.y + height/2);

	// set centre's height : average corners' height, then add random value	
	float centerHeight = ((getHeightAt(topLeftCoords) +  getHeightAt(topRight) + getHeightAt(bottomLeft) + getHeightAt(bottomRight)) / 4.0) + getRandomInRange(hRange) * scale;
	setHeightAt(squareCenter, centerHeight);

	// (Square Step)
	// find the centres of the 4 diamonds
	glm::vec2 middleLeft = glm::vec2(topLeftCoords.x, topLeftCoords.y + height/2);
	glm::vec2 topMiddle = glm::vec2(topLeftCoords.x + width/2, topLeftCoords.y);
	glm::vec2 middleRight = glm::vec2(topLeftCoords.x + width, topLeftCoords.y + height/2);
	glm::vec2 bottomMiddle = glm::vec2(topLeftCoords.x + width/2, topLeftCoords.y + height);

	// compute each centre's height : average corner's height, then add random value		
	float middleLeftHeight		= ((centerHeight + heights2D[topLeftCoords.x][topLeftCoords.y] + heights2D[bottomLeft.x][bottomLeft.y]) / 3.0) + getRandomInRange(hRange) * scale;
	float topMiddleHeight		= ((centerHeight + heights2D[topLeft.x][topLeft.y] + heights2D[topRight.x][topRight.y]) / 3.0) + getRandomInRange(hRange) * scale;
	float middleRightHeight		= ((centerHeight + heights2D[topRight.x][topRight.y] + heights2D[bottomRight.x][bottomRight.y]) / 3.0) + getRandomInRange(hRange) * scale;
	float bottomMiddleHeight	= ((centerHeight + heights2D[bottomLeft.x][bottomLeft.y] + heights2D[bottomRight.x][bottomRight.y]) / 3.0) + getRandomInRange(hRange) * scale;

	// set each centre's height 
	setHeightAt(middleLeft, middleLeftHeight);
	setHeightAt(topMiddle, topMiddleHeight);
	setHeightAt(middleRight, middleRightHeight);
	setHeightAt(bottomMiddle, bottomMiddleHeight);
	
	
	// set dimensions for quadrants (account for the off-by-one prevention as well)
	width = width++ / 2;
	height = height++ / 2;

	//reduce range of random increment
	hRange /= 2;

	// recurse on quadrants
	generateDiamondSquare(topLeftCoords, width, height, hRange, scale);
	generateDiamondSquare(topMiddle, width, height, hRange, scale);
	generateDiamondSquare(middleLeft, width, height, hRange, scale);
	generateDiamondSquare(squareCenter, width, height, hRange, scale);
}

void HeightMap::generate()
{	
	for (int z = 0; z < HEIGHT; z++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			// generate color and store as texture data
			//float R = perlinNoise.noise(float(x), float(z), 0.8);
			float R = fBm(
				glm::vec2(float(x), float(z)),
				2.0,
				2.0,
				2
			);
			/*float R = HybridMultifractal(
				glm::vec2(float(x), float(z)),
				1.0,
				2.0,
				11,
				0.0);*/
			rgbData.push_back(glm::vec3(R, baseColor.g, baseColor.b));
		}
	}
}

void HeightMap::generateTestHeightMap()
{
	float numSteps = 5;
	for (int x = 0; x < WIDTH; x++)
	{
		int xRange = WIDTH / numSteps;
		float xInterval = (1.0 / numSteps) / 2;
		float xH = (xInterval * (x / xRange));
		
		for (int z = 0; z < HEIGHT; z++)
		{
			int zRange = HEIGHT / numSteps;
			float zInterval = (1.0 / numSteps) / 2;
			float zH = (zInterval * (z / zRange));

			rgbData.push_back(glm::vec3(xH + zH, 0.0, 0.0));
		}
	}
}

double HeightMap::fBm(glm::vec2 point, double H, float lacunarity, float octaves)
{
	const int MAX_OCTAVES = 12;
	double value, frequency, remainder;
	int i;
	bool first = true;
	double exponent_array[MAX_OCTAVES];
	
	if (first) 
	{
		frequency = 1.0;
		for (i = 0; i<MAX_OCTAVES; i++)
		{			
			exponent_array[i] = pow(frequency, -H);
			frequency *= lacunarity;
		}

		first = false;
	}

	value = 0.0;
	for (i = 0; i<octaves; i++) 
	{
		value += perlinNoise.noise(point.x, point.y, 0.8) * exponent_array[i];
		point.x *= lacunarity;
		point.y *= lacunarity;
	} 

	remainder = octaves - (int)octaves;

	if (remainder)
	{
		value += remainder * perlinNoise.noise(point.x, point.y, 0.8) * exponent_array[i];
	}

	return(value);
}

double HeightMap::HybridMultifractal(glm::vec2 point, double H, double lacunarity, double octaves, double offset)
{
	double frequency, result, signal, weight, remainder;
	double Noise3();
	int i;
	bool first = true;
	static double *exponent_array;
	
	if (first) 
	{		
		exponent_array = (double *)malloc(octaves * sizeof(double));
		frequency = 1.0;

		for (i = 0; i<octaves; i++) 
		{
			
			exponent_array[i] = pow(frequency, -H);
			frequency *= lacunarity;
		}

		first = false;
	}
	
	result = (perlinNoise.noise(point.x, point.y, 0.8) + offset) * exponent_array[0];

	weight = result;
	
	point.x *= lacunarity;
	point.y *= lacunarity;
	
	for (i = 1; i<octaves; i++) 
	{		
		if (weight > 1.0) weight = 1.0;
		
		signal = (perlinNoise.noise(point.x, point.y, 0.8) + offset) * exponent_array[i];
		
		result += weight * signal;
		
		weight *= signal;
		
		point.x *= lacunarity;
		point.y *= lacunarity;
	} 
	  
	remainder = octaves - (int)octaves;
	if (remainder)
	{
		result += remainder * perlinNoise.noise(point.x, point.y, 0.8) * exponent_array[i];
	}

	return(result);
} 

/*
*	Returns a random float between [-range, +range]
*/
float HeightMap::getRandomInRange(float range)
{
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float rInRange = (r * 2 * range) - range;
	return rInRange;
}

void HeightMap::setHeightAt(glm::vec2 coords, float height)
{
	if (coords.x < 0 ||
		coords.y < 0 ||
		coords.x >= WIDTH ||
		coords.y >= HEIGHT)
	{
		std::cout << "Cannot set height at " << coords.x << ", " << coords.y << std::endl;
		return;
	}

	heights2D[coords.x][coords.y] = height;
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

	/*std::vector<float> heightMap = std::vector<float>();
	for (int z = 0; z < HEIGHT; z++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			float height = heights2D[x][z];
			heightMap.push_back(height);
		}
	}
	return heightMap;*/

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
		uint8_t rByte = uint8_t(255*color.r);
		rgbBytes.push_back(rByte);
		
		// make sure that the red channel wasn't specifically requested
		if (format != GL_RED)
		{
			uint8_t gByte = uint8_t(255 * color.g);
			uint8_t bByte = uint8_t(255 * color.b);
			rgbBytes.push_back(gByte);
			rgbBytes.push_back(bByte);
		}

		// check if alpha format requested
		if (format == GL_RGBA)
		{
			uint8_t aByte = 255;
			rgbBytes.push_back(aByte);
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
	std::vector<unsigned char> pixels = getAsByteVector(GL_RGB);
	if (stbi_write_png(filename.c_str(), WIDTH, HEIGHT, colorDepth, pixels.data(), stride) == 0)
	{
		std::cout << "Failed to write to: " << filename.c_str() << std::endl;
		return;
	}
}