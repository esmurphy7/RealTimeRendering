#pragma once
#include <glm\glm.hpp>
#include "InputHandler.h"

class Camera
{
private:
	const float MAX_FOV = 80.0f;
	const float MIN_FOV = 20.0f;
	float initialFoV;	
	float horizontalAngle;
	float verticalAngle;

public:
	float FoV;
	float aspectRatio;	
	float nearClip;
	float farClip;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up;
	glm::vec3 right;

	Camera(float x, float y, float z);
	Camera(glm::vec3 origin);	
	void update(InputData, float);
};

Camera::Camera(float x, float y, float z) : Camera(glm::vec3(x, y, z)) { }

Camera::Camera(glm::vec3 origin)
{
	initialFoV = 45.0f;
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;

	nearClip = 0.1;
	farClip = 1000.0f;
	position = origin;
}

void Camera::update(InputData inputData, float deltaTime)
{
	// compute horizontal and vertical angle
	horizontalAngle += inputData.mouseSpeed * deltaTime * float(inputData.windowWidth / 2 - inputData.mouseX);
	verticalAngle += inputData.mouseSpeed * deltaTime * float(inputData.windowHeight / 2 - inputData.mouseY);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector : perpendicular to both direction and right
	up = glm::cross(right, direction);

	// compute Fov restricted within range
	float targetFoV = initialFoV - 5 * inputData.mouseWheel;
	if (targetFoV <= MAX_FOV && targetFoV >= MIN_FOV)
	{
		FoV = targetFoV;
	}

	// compute aspect ratio
	aspectRatio = float(inputData.windowWidth) / float(inputData.windowHeight);

	// update the position vector based on arrow key presses
	glm::vec3 deltaPosition = glm::vec3();
	if (inputData.keyChangeX)
	{
		deltaPosition += right * inputData.keyChangeX;
	}
	if (inputData.keyChangeY)
	{
		deltaPosition += direction * inputData.keyChangeY;
	}
	position += deltaPosition;
}