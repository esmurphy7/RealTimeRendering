#pragma once
#include <stdio.h>
#include <SDL.h>

struct InputData
{	
	float keySpeed;
	float mouseSpeed;
	float keyChangeX, keyChangeY;
	float mouseX, mouseY;
	float mouseWheel;
	float windowWidth;
	float windowHeight;	
};

class InputHandler
{
private:
	const float KEY_SPEED = 10.0f;	
	const float MOUSE_SPEED = 0.5f;
	const float MAX_MOUSE_WHEEL = 5.0f;
	const float MIN_MOUSE_WHEEL = -5.0f;
	SDL_Window* window;
	InputData inputData;

public:
	InputHandler(SDL_Window*);
	InputData getInputData();
	int updateInput(float);
};

InputHandler::InputHandler(SDL_Window* window)
{
	this->window = window;
}

InputData InputHandler::getInputData()
{
	return inputData;
}

int InputHandler::updateInput(float deltaTime)
{
	// get window dimensions
	int windowW, windowH;
	SDL_GetWindowSize(window, &windowW, &windowH);
	inputData.windowWidth = windowW;
	inputData.windowHeight = windowH;

	// get current mouse position
	int x, y;
	SDL_GetMouseState(&x, &y);
	inputData.mouseX = float(x);
	inputData.mouseY = float(y);	

	// pass constants into input data
	inputData.keySpeed = KEY_SPEED;
	inputData.mouseSpeed = MOUSE_SPEED;

	// Handle all events since the last update
	float deltaMouseWheel = 0;
	float deltaX = 0;
	float deltaY = 0;
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
		{
			return -1;
		}

		if (ev.type == SDL_KEYDOWN)
		{
			if (ev.key.keysym.sym == SDLK_RIGHT)
			{
				deltaX += deltaTime * KEY_SPEED;
			}
			if (ev.key.keysym.sym == SDLK_LEFT)
			{
				deltaX -= deltaTime * KEY_SPEED;
			}
			if (ev.key.keysym.sym == SDLK_UP)
			{
				deltaY += deltaTime * KEY_SPEED;
			}
			if (ev.key.keysym.sym == SDLK_DOWN)
			{
				deltaY -= deltaTime * KEY_SPEED;
			}
		}

		inputData.keyChangeX = deltaX;
		inputData.keyChangeY = deltaY;

		if (ev.type == SDL_MOUSEWHEEL)
		{
			deltaMouseWheel = ev.wheel.y;
		}
	}

	// only accumulate mouse wheel moves within a range
	if (inputData.mouseWheel + deltaMouseWheel <= MAX_MOUSE_WHEEL && inputData.mouseWheel + deltaMouseWheel >= MIN_MOUSE_WHEEL)
	{
		inputData.mouseWheel += deltaMouseWheel;
	}	

	// reset mouse position to center of screen
	SDL_WarpMouseInWindow(window, windowW / 2, windowH / 2);

	return 0;
}

