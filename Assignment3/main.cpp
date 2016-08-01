#include <stdio.h>

#include <SDL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>

#include "opengl.h"
#include "shaderset.h"
#include "tiny_obj_loader.h"
#include "InputHandler.h"
#include "Camera.h"
#include "TerrainMesh.h"
#include "Skybox.h"
#include "CubicBezierCurve.h"
#include "BezierPath.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

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

	//======================== VAO ===============================
	// Hook up the vertex and index buffers to a "vertex array object" (VAO)
	// VAOs are the closest thing OpenGL has to a "mesh" object.
	// VAOs are used to feed data from buffers to thgle inputs of a vertex shader.
	//============================================================	

	//===================== VBO/EBO ===============================
	Skybox skyBox = Skybox();
	skyBox.load();

	TerrainMesh terrainMesh = TerrainMesh(64, 64, 0.0);
	terrainMesh.load();

	
	//============================================================

	//===================== TEXTURES =============================
	
	//============================================================

	//======================= SHADERS ============================
	// init terrain shader
	ShaderSet terrainShaderSet;
	terrainShaderSet.SetVersion("330");

	// set uniforms (use preambles here or define them in the shader file, but NOT both)
	terrainShaderSet.SetPreamble(
		"uniform mat4 iModelViewProjection;\n"
		"uniform mat4 iModel;\n"
		"uniform mat4 iView;\n"
		"uniform vec3 iLightPosition_worldspace;\n"
		"uniform sampler2DArray iTextureArray;\n"
	);
	GLuint* terrainShaderId = terrainShaderSet.AddProgramFromExts({ "a3.vert", "a3.frag" });


	// init skybox shader
	ShaderSet skyboxShaderSet;
	skyboxShaderSet.SetVersion("330");

	// set uniforms (use preambles here or define them in the shader file, but NOT both)
	skyboxShaderSet.SetPreamble(
		"uniform mat4 iProjection;\n"
		"uniform mat4 iView;\n"
		"uniform samplerCube iSkyBoxCubeTexture;\n"
	);
	GLuint* skyboxShaderId = skyboxShaderSet.AddProgramFromExts({ "skybox.vert", "skybox.frag" });
	//============================================================

	//======================== LIGHTS ============================
	glm::vec3 light = glm::vec3(4, 40, 4);
	//============================================================

	//================== BEZIER CURVES ===========================
	glm::vec3 xzLength = glm::vec3(32.0, 0.0, 32.0);
	glm::vec3 yPull = glm::vec3(0.0, 20.0, 0.0);
	glm::vec3 xPull = glm::vec3(20.0, 0.0, 0.0);

	CubicBezierCurve bezierCurve1 = CubicBezierCurve(light, xzLength, yPull, yPull);
	CubicBezierCurve bezierCurve2 = CubicBezierCurve(bezierCurve1.p3, xzLength, -yPull, -yPull);
	CubicBezierCurve bezierCurve3 = CubicBezierCurve(bezierCurve2.p3, -xzLength, xPull, xPull);
	CubicBezierCurve bezierCurve4 = CubicBezierCurve(bezierCurve3.p3, -xzLength, -xPull, -xPull);
	
	BezierPath bezierPath = BezierPath(std::vector<CubicBezierCurve>{ bezierCurve1, bezierCurve2, bezierCurve3, bezierCurve4 });
	//============================================================
	
    // Begin main loop
	double lastTime = 0;
	InputHandler inputHandler = InputHandler(window);
	Camera camera = Camera(4, 40, 4);
	float bezierTime = 0.0;
	const float bezierSpeed = 0.001;
    while (1)
    {			
		//================= UPDATE USER INPUT ========================
		double currentTime = SDL_GetTicks() / 1000.0;		
		float deltaTime = float(currentTime - lastTime);
		
		if (inputHandler.updateInput(deltaTime) == -1)
		{
			goto quit;
		}
		camera.update(inputHandler.getInputData(), deltaTime);

		lastTime = currentTime;
		//============================================================

		//================= COMPUTE MATRICES =========================
		// move camera position along bezier curve		
		glm::vec3 bezPos = bezierPath.getPointAt(bezierTime);
		bezierTime += bezierSpeed;
		bezierTime = (bezierTime > 1.0) ? 0.0 : bezierTime;
		camera.position = bezPos;

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
		glm::mat4 ModelViewProjection = Projection * View * Model; // Remember, matrix multiplication is the other way around
		//glm::mat4 ModelViewProjection = MVP;
		//============================================================		

		//================== UPDATE SHADERS ==========================
		// Set the color to clear with
		//glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//===================== Z-BUFFER =============================
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);


		// TERRAIN ======================================================
		// Recompile/relink any programs that changed (must be called)
		terrainShaderSet.UpdatePrograms();

		// set OpenGL's shader program (must be called in loop)
		glUseProgram(*terrainShaderId);

		// get uniform handles
		GLuint iModelViewProjectionLoc = glGetUniformLocation(*terrainShaderId, "iModelViewProjection");
		GLuint iModelLoc = glGetUniformLocation(*terrainShaderId, "iModel");
		GLuint iViewLoc = glGetUniformLocation(*terrainShaderId, "iView");
		GLuint iLightPosition_worldspaceLoc = glGetUniformLocation(*terrainShaderId, "iLightPosition_worldspace");

		// send matrix uniforms to shader
		if (iModelViewProjectionLoc != -1)
		{
			glUniformMatrix4fv(iModelViewProjectionLoc, 1, GL_FALSE, &ModelViewProjection[0][0]);
		}

		if (iModelLoc != -1)
		{
			glUniformMatrix4fv(iModelLoc, 1, GL_FALSE, &Model[0][0]);
		}
		if (iViewLoc != -1)
		{
			glUniformMatrix4fv(iViewLoc, 1, GL_FALSE, &View[0][0]);
		}						

		// pass light position uniform to shader
		if (iLightPosition_worldspaceLoc != -1) 
		{
			glUniform3f(iLightPosition_worldspaceLoc, light.x, light.y, light.z);
		}				

		terrainMesh.generateTextureUniform(terrainShaderId, "iTextureArray");

		terrainMesh.attachToVAO(0, 1, 2, 3);

		terrainMesh.draw();



		// SKYBOX ========================================================
		// Recompile/relink any programs that changed (must be called)
		skyboxShaderSet.UpdatePrograms();

		// set OpenGL's shader program (must be called in loop)
		glUseProgram(*skyboxShaderId);

		// get skybox shader uniform handles
		GLuint iSkyBoxViewLoc = glGetUniformLocation(*skyboxShaderId, "iView");
		GLuint iSkyBoxProjectionLoc = glGetUniformLocation(*skyboxShaderId, "iProjection");
		if (iSkyBoxViewLoc != -1)
		{
			// remove the translation component of the view matrix
			// to make sure the skybox's origin is always at the camera
			glm::mat4 newView = glm::mat4(glm::mat3(View));
			glUniformMatrix4fv(iSkyBoxViewLoc, 1, GL_FALSE, &newView[0][0]);
		}
		if (iSkyBoxProjectionLoc != -1)
		{
			glUniformMatrix4fv(iSkyBoxProjectionLoc, 1, GL_FALSE, &Projection[0][0]);
		}

		// generate uniforms for object textures		
		skyBox.generateTextureUniform(skyboxShaderId, "iSkyBoxCubeTexture");

		skyBox.attachToVAO(4);

		skyBox.draw();


		// draw wireframe of mesh
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		
		

		// check for errors
		GLenum err2;
		while ((err2 = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err2 << std::endl;
		}

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