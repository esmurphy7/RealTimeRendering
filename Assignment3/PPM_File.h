//
// Created by Evan on 5/18/2016.
//

#ifndef CSC305_A1_PPM_FILE_H
#define CSC305_A1_PPM_FILE_H

#include <fstream>
#include <glm\glm.hpp>

class PPM_File
{
public:
	std::string fileName;
	PPM_File& operator=(const PPM_File&) = delete;
	void open(std::string fileName, unsigned int imgWidth, unsigned int imgHeight);
	void close();
	void writeColor(glm::vec3 color);
	void writeColor(float r, float g, float b);

private:
	std::ofstream file;
};

void PPM_File::open(std::string fileName, unsigned int imgWidth, unsigned int imgHeight)
{
	this->fileName = fileName;
	file.open(fileName);
	file << "P3\n" << imgWidth << " " << imgHeight << "\n255\n";
}

void PPM_File::close()
{
	file.close();
}

void PPM_File::writeColor(glm::vec3 color)
{
	writeColor(color.r, color.g, color.b);
}

void PPM_File::writeColor(float r, float g, float b)
{
	int ir = int(255.99*r);
	int ig = int(255.99*g);
	int ib = int(255.99*b);
	file << ir << " " << ig << " " << ib << "\n";
}

#endif //CSC305_A1_PPM_FILE_H
