#include <glm\glm.hpp>

class CubicBezierCurve
{	

public:
	glm::vec3 p0, p1, p2, p3;

	CubicBezierCurve(glm::vec3 start, glm::vec3 length, glm::vec3 startPull, glm::vec3 endPull);
	CubicBezierCurve(std::vector<glm::vec3> controlPoints);
	glm::vec3 getPointAt(float t);

};

CubicBezierCurve::CubicBezierCurve(glm::vec3 start, glm::vec3 length, glm::vec3 startPull, glm::vec3 endPull)
{
	p0 = start;
	p1 = p0;
	p1 += startPull;
	p3 = p0;
	p3 += length;
	p2 = p3;
	p2 += endPull;
}

CubicBezierCurve::CubicBezierCurve(std::vector<glm::vec3> controlPoints)
{
	if (controlPoints.size() != 4)
	{
		std::cout << "Cannot create CubicBezierCurve from " << controlPoints.size() << " control points" << std::endl;
		return;
	}

	p0 = controlPoints.at(0);
	p1 = controlPoints.at(1);
	p2 = controlPoints.at(2);
	p3 = controlPoints.at(3);
}

glm::vec3 CubicBezierCurve::getPointAt(float t)
{
	float u = 1.0 - t;
	float tt = t*t;
	float uu = u*u;
	float uuu = uu * u;
	float ttt = tt * t;

	glm::vec3 p = uuu * p0;
	p += 3 * uu * t * p1; 
	p += 3 * u * tt * p2; 
	p += ttt * p3;

	return p;
}