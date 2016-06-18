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

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

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
		"uniform sampler2D iTextureSampler;\n"
	);

	// define shader program from vertex and fragment shader files
	GLuint* shaderId = shaders.AddProgramFromExts({ "a2.vert", "a2.frag" });	
	//============================================================

	//======================== VAO ===============================
	// Hook up the vertex and index buffers to a "vertex array object" (VAO)
	// VAOs are the closest thing OpenGL has to a "mesh" object.
	// VAOs are used to feed data from buffers to thgle inputs of a vertex shader.
	GLuint meshVAO;
	glGenVertexArrays(1, &meshVAO);
	//============================================================	

	//===================== VBO/EBO ===============================
	// Load the mesh and its materials
	std::string meshObj = "models/cube/cube.obj";
	std::string objBase = "models/cube/";
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	if (!tinyobj::LoadObj(shapes, materials, err, meshObj.c_str(), objBase.c_str()))
	{
		fprintf(stderr, "Failed to load cube.obj: %s\n", err.c_str());
		return 1;
	}

	GLuint positionVBO = 0;
	GLuint texcoordVBO = 0;
	GLuint normalVBO = 0;
	GLuint indicesEBO = 0;

	// Upload per-vertex positions
	if (!shapes[0].mesh.positions.empty())
	{
		glGenBuffers(1, &positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
		glBufferData(GL_ARRAY_BUFFER, shapes[0].mesh.positions.size() * sizeof(float), shapes[0].mesh.positions.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Upload per-vertex texture coordinates
	if (!shapes[0].mesh.texcoords.empty())
	{
		glGenBuffers(1, &texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, shapes[0].mesh.texcoords.size() * sizeof(float), shapes[0].mesh.texcoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Upload per-vertex normals
	if (!shapes[0].mesh.normals.empty())
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, shapes[0].mesh.normals.size() * sizeof(float), shapes[0].mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Upload the indices that form triangles
	if (!shapes[0].mesh.indices.empty())
	{
		glGenBuffers(1, &indicesEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[0].mesh.indices.size() * sizeof(unsigned int), shapes[0].mesh.indices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}	  
	//============================================================

	//===================== TEXTURES =============================
	// get name of texture image from loaded obj
	int materialId = shapes[0].mesh.material_ids[0];
	//std::string textureImage = materials[materialId].diffuse_texname;
	std::string textureImage = "\models\\cube\\default.png";

	// load the texture image
	int imgWidth;
	int imgHeight;
	int nColorDepth;
	unsigned char* pixels = stbi_load(textureImage.c_str(), &imgWidth, &imgHeight, &nColorDepth, 0);

	if (pixels == NULL)
	{
		fprintf(stderr, "Failed to load %s\n", textureImage.c_str());
		return 0;
	}

	// generate and bind texture object
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	

	// upload texture data to OpenGL
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		imgWidth,
		imgHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels);

	// set filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);	
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
		
		if (inputHandler.updateInput(deltaTime) == -1)
		{
			goto quit;
		}
		camera.update(inputHandler.getInputData(), deltaTime);

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
		glm::mat4 ModelViewProjection = Projection * View * Model; // Remember, matrix multiplication is the other way around
		//glm::mat4 ModelViewProjection = MVP;
		//============================================================

		//================== UPDATE SHADERS ==========================
		// Recompile/relink any programs that changed (must be called)
		shaders.UpdatePrograms();

		// get uniform handles
		GLuint iModelViewProjectionLoc = glGetUniformLocation(*shaderId, "iModelViewProjection");
		GLuint iTextureSamplerLoc = glGetUniformLocation(*shaderId, "iTextureSampler");

		// Send our transformation to the currently bound shader, in the "iModelViewProjection" uniform
		// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
		if (iModelViewProjectionLoc != -1)
		{
			glUniformMatrix4fv(iModelViewProjectionLoc, 1, GL_FALSE, &ModelViewProjection[0][0]);
		}

		// activate texture channel 0 and pass it to shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		if (iTextureSamplerLoc != -1)
		{
			glUniform1i(iTextureSamplerLoc, 0);
		}

		// set OpenGL's shader program (must be called in loop)
		glUseProgram(*shaderId);
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
		// Attach position buffer as attribute 0
		if (positionVBO != 0)
		{
			glBindVertexArray(meshVAO);

			// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
			// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
			glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
			glEnableVertexAttribArray(0);

			glBindVertexArray(0);
		}

		// Attach texcoord buffer as attribute 1
		if (texcoordVBO != 0)
		{
			glBindVertexArray(meshVAO);

			// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
			// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
			glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);
		}

		// Attach normal buffer as attribute 2
		if (normalVBO != 0)
		{
			glBindVertexArray(meshVAO);

			// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
			// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
			glEnableVertexAttribArray(2);

			glBindVertexArray(0);
		}

		// attach the index EBO to the mesh VAO
		if (indicesEBO != 0)
		{
			glBindVertexArray(meshVAO);

			// Note: Calling glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); when a VAO is bound attaches the index buffer to the VAO.
			// From an API design perspective, this is subtle.
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);

			glBindVertexArray(0);
		}

		// Can now bind the vertex array object to the graphics pipeline, to render with it.
		// For example:
		glBindVertexArray(meshVAO);
		//============================================================

		// Draw the vertices
		glDrawElements(
			GL_TRIANGLES,      // mode
			shapes[0].mesh.indices.size(),    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

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