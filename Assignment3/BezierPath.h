#include <vector>

class BezierPath
{
private:
	std::vector<CubicBezierCurve> bezierCurves = std::vector<CubicBezierCurve>();

public:
	BezierPath(std::vector<CubicBezierCurve> bezierCurves);
	glm::vec3 getPointAt(float t);
};

BezierPath::BezierPath(std::vector<CubicBezierCurve> bezierCurves)
{
	this->bezierCurves = bezierCurves;
}

glm::vec3 BezierPath::getPointAt(float t)
{
	// determine which curve in the path to traverse
	float range = 1.0 / bezierCurves.size();
	int curveIndex = int(t / range);	

	// map t back to [0.0 - 1.0] range
	float newT = (t - (range * curveIndex)) / range;

	// get the point on the correct curve
	CubicBezierCurve curve = bezierCurves.at(curveIndex);
	glm::vec3 point = curve.getPointAt(newT);

	return point;
}
