#include <glm\glm.hpp>

class CubicBezierCurve
{	

public:
	glm::vec3 p0, p1, p2, p3;

	CubicBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	glm::vec3 getPointAt(float t);

};

CubicBezierCurve::CubicBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	this->p0 = p0;
	this->p1 = p1;
	this->p2 = p2;
	this->p3 = p3;
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