#include <stdio.h>

#include <SDL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

#include "opengl.h"
#include "shaderset.h"
#include "InputHandler.h"
#include "Camera.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 30.0f;
float mouseSpeed = 0.005f;
float mouseWheelPos = 0;

void computeMatricesFromInputs(SDL_Window *window, float deltaTime) 
{
	// Get mouse position
	int xpos, ypos;
	SDL_GetMouseState(&xpos, &ypos);
	float mouseX = float(xpos);
	float mouseY = float(ypos);		

	// Compute new orientation
	float dW = float(WINDOW_WIDTH / 2 - mouseX);
	float dH = float(WINDOW_HEIGHT / 2 - mouseY);
	horizontalAngle += mouseSpeed * dW;
	verticalAngle += mouseSpeed * dH;

	//fprintf(stdout, "horizontalAngle, verticalAngle: %f, %f\n", horizontalAngle, verticalAngle);
	//fprintf(stdout, "dW, dH: %f, %f\n", dW, dH);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
		{
			break;
		}

		if (ev.type == SDL_KEYDOWN)
		{
			if (ev.key.keysym.sym == SDLK_RIGHT)
			{
				position += right * deltaTime * speed;
			}
			if (ev.key.keysym.sym == SDLK_LEFT)
			{
				position -= right * deltaTime * speed;
			}
			if (ev.key.keysym.sym == SDLK_UP)
			{
				position += direction * deltaTime * speed;
			}
			if (ev.key.keysym.sym == SDLK_DOWN)
			{
				position -= direction * deltaTime * speed;
			}
		}

		if (ev.type == SDL_MOUSEWHEEL)
		{
			mouseWheelPos = ev.wheel.y;
		}
	}

	float FoV = initialFoV - 5 * mouseWheelPos; // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, float(WINDOW_WIDTH)/float(WINDOW_HEIGHT), 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// reset mouse position to center of screen
	SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}


// SDL wants main() to have this exact signature
extern "C" int main(int argc, char* argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    // Plain 32-bit RGBA8 pixel format
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // Don't need built-in depth buffer, we use our own.
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    // Select OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    // Create the window
	SDL_Window* window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    // Create the OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context)
    {
        fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize OpenGL (must be called before calling any OpenGL functions)
    OpenGL_Init();	

	// DEBUG: print OpenGL version
	const GLubyte* version = glGetString(GL_VERSION);
	fprintf(stdout, "OpenGL Version: %s\n", version);

	//======================= SHADERS ============================
	// init shaderset construct
	ShaderSet shaders;
	shaders.SetVersion("330");

	// set uniforms (use preambles here or define them in the shader file, but NOT both)
	shaders.SetPreamble(
		"uniform mat4 iModelViewProjection;\n"
	);

	// define shader program from vertex and fragment shader files
	GLuint* program = shaders.AddProgramFromExts({ "a2.vert", "a2.frag" });	
	//============================================================

	//======================== VAO ===============================
	// generate VAO reference
	GLuint vertexVAO;
	glGenVertexArrays(1, &vertexVAO);

	// make the VAO active
	glBindVertexArray(vertexVAO);
	//============================================================	

	//===================== VBO ==================================
	// define vertex array
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat vertices[] = {
		-1.0f, -1.0f, -1.0f, // triangle 1 : begin
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f, -1.0f, // triangle 2 : begin
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f, // triangle 2 : end
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	int numVertices = 12 * 3;

	// One color for each vertex. They were generated randomly.
	static const GLfloat colors[] = {
		0.583f, 0.771f, 0.014f,
		0.609f, 0.115f, 0.436f,
		0.327f, 0.483f, 0.844f,
		0.822f, 0.569f, 0.201f,
		0.435f, 0.602f, 0.223f,
		0.310f, 0.747f, 0.185f,
		0.597f, 0.770f, 0.761f,
		0.559f, 0.436f, 0.730f,
		0.359f, 0.583f, 0.152f,
		0.483f, 0.596f, 0.789f,
		0.559f, 0.861f, 0.639f,
		0.195f, 0.548f, 0.859f,
		0.014f, 0.184f, 0.576f,
		0.771f, 0.328f, 0.970f,
		0.406f, 0.615f, 0.116f,
		0.676f, 0.977f, 0.133f,
		0.971f, 0.572f, 0.833f,
		0.140f, 0.616f, 0.489f,
		0.997f, 0.513f, 0.064f,
		0.945f, 0.719f, 0.592f,
		0.543f, 0.021f, 0.978f,
		0.279f, 0.317f, 0.505f,
		0.167f, 0.620f, 0.077f,
		0.347f, 0.857f, 0.137f,
		0.055f, 0.953f, 0.042f,
		0.714f, 0.505f, 0.345f,
		0.783f, 0.290f, 0.734f,
		0.722f, 0.645f, 0.174f,
		0.302f, 0.455f, 0.848f,
		0.225f, 0.587f, 0.040f,
		0.517f, 0.713f, 0.338f,
		0.053f, 0.959f, 0.120f,
		0.393f, 0.621f, 0.362f,
		0.673f, 0.211f, 0.457f,
		0.820f, 0.883f, 0.371f,
		0.982f, 0.099f, 0.879f
	};

	// Generate vertex VBO reference
	GLuint vertexbuffer;	
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// generate color VBO reference
	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	//============================================================

    // Begin main loop
	double lastTime = 0;
	InputHandler inputHandler = InputHandler(window);
	Camera camera = Camera();
    while (1)
    {		
		//================= UPDATE USER INPUT ========================
		double currentTime = SDL_GetTicks() / 1000.0;		
		float deltaTime = float(currentTime - lastTime);
		/*
		inputHandler.updateInput(deltaTime);
		camera.FoV			= inputHandler.FoV;
		camera.aspectRatio	= inputHandler.aspectRatio;
		camera.position		= inputHandler.position;
		camera.position		= inputHandler.direction;
		camera.right		= inputHandler.right;
		camera.up			= inputHandler.up;
		*/

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(window, deltaTime);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		lastTime = currentTime;
		//============================================================

		//================= COMPUTE MATRICES =========================
		// Projection matrix
		glm::mat4 Projection = glm::perspective(
			camera.FoV,
			camera.aspectRatio,
			camera.nearClip,
			camera.farClip
		);

		// Or, for an ortho camera :
		//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

		// Camera matrix
		glm::mat4 View = glm::lookAt(
			camera.position,
			camera.position + camera.direction,
			camera.up
		);

		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);
		// Our ModelViewProjection : multiplication of our 3 matrices
		//glm::mat4 ModelViewProjection = Projection * View * Model; // Remember, matrix multiplication is the other way around
		glm::mat4 ModelViewProjection = MVP;
		//============================================================

		//================== UPDATE SHADERS ==========================
		// Recompile/relink any programs that changed (must be called)
		shaders.UpdatePrograms();

		// Get a handle for our "iModelViewProjection" uniform
		// Only during the initialisation
		GLuint iModelViewProjectionLoc = glGetUniformLocation(*program, "iModelViewProjection");

		// Send our transformation to the currently bound shader, in the "iModelViewProjection" uniform
		// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
		if (iModelViewProjectionLoc != -1)
		{
			glUniformMatrix4fv(iModelViewProjectionLoc, 1, GL_FALSE, &ModelViewProjection[0][0]);
		}

		// set OpenGL's shader program (must be called in loop)
		glUseProgram(*program);
		//============================================================
		
        // Set the color to clear with
        glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//===================== Z-BUFFER =============================
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
		//============================================================

		//================ VAO ATTRIBUTES ===========================
		// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
		// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).		

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);		

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                  // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);		
		//============================================================

		// Draw the vertices
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

		// disable the attribute after drawing
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// disable the depth buffer
		glDisable(GL_DEPTH_TEST);

        // SDL docs: "On Mac OS X make sure you bind 0 to the draw framebuffer before swapping the window, otherwise nothing will happen."
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        // Display the frame of rendering to the window
        SDL_GL_SwapWindow(window);		
    }

quit:
    SDL_Quit();
    return 0;
}