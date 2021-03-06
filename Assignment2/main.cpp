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
		"uniform mat4 iModel;\n"
		"uniform mat4 iView;\n"
		"uniform sampler2D iTextureSampler;\n"
		"uniform vec3 iLightPosition_worldspace;\n"
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
	//std::string meshObj = "models/cube/cube.obj";
	//std::string objBase = "models/cube/";
	//std::string meshObj = "models/tinyobjloader/test-nan.obj";
	//std::string objBase = "models/tinyobjloader/";
	//std::string meshObj = "models/wedge/wedge.obj";
	//std::string objBase = "models/wedge/";
	std::string meshObj = "models/test/multiple.obj";
	std::string objBase = "models/test/";

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(shapes, materials, err, meshObj.c_str(), objBase.c_str()))
	{
		fprintf(stderr, "Failed to load cube.obj: %s\n", err.c_str());
		return 1;
	}	

	// find the total sizes of all shapes' data
	size_t vertex_buffer_size = 0;
	size_t texCoords_buffer_size = 0;
	size_t normals_buffer_size = 0;
	size_t indices_buffer_size = 0;
	for each (tinyobj::shape_t shape in shapes) 
	{
		vertex_buffer_size += sizeof(float)* shape.mesh.positions.size();
		texCoords_buffer_size += sizeof(float)* shape.mesh.texcoords.size();
		normals_buffer_size += sizeof(float)* shape.mesh.normals.size();
		indices_buffer_size += sizeof(float)* shape.mesh.indices.size();
	}

	// initialize vertex VBO
	GLuint positionVBO = 0;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// initialize UVcoords VBO
	GLuint texcoordVBO = 0;
	glGenBuffers(1, &texcoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
	glBufferData(GL_ARRAY_BUFFER, texCoords_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// initialize normals VBO
	GLuint normalVBO = 0;
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, normals_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// initialize indices EBO
	GLuint indicesEBO = 0;
	glGenBuffers(1, &indicesEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// upload VBO and EBO data for each shape loaded
	size_t vertex_buffer_offset = 0;
	size_t texCoords_buffer_offset = 0;
	size_t normals_buffer_offset = 0;
	size_t indices_buffer_offset = 0;
	for each (tinyobj::shape_t shape in shapes)
	{
		// Upload per-vertex positions
		if (!shape.mesh.positions.empty())
		{				
			glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
			glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, sizeof(float)* shape.mesh.positions.size(), &shape.mesh.positions[0]);
			vertex_buffer_offset += sizeof(float)* shape.mesh.positions.size();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// Upload per-vertex texture coordinates
		if (!shape.mesh.texcoords.empty())
		{
			glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
			glBufferSubData(GL_ARRAY_BUFFER, texCoords_buffer_offset, sizeof(float) * shape.mesh.texcoords.size(), &shape.mesh.texcoords[0]);
			texCoords_buffer_offset += sizeof(float) * shape.mesh.texcoords.size();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// Upload per-vertex normals
		if (!shape.mesh.normals.empty())
		{
			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glBufferSubData(GL_ARRAY_BUFFER, normals_buffer_offset, sizeof(float) * shape.mesh.normals.size(), &shape.mesh.normals[0]);
			normals_buffer_offset += sizeof(float) * shape.mesh.normals.size();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// Upload the indices that form triangles
		if (!shape.mesh.indices.empty())
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_offset, sizeof(unsigned int) * shape.mesh.indices.size(), &shape.mesh.indices[0]);
			indices_buffer_offset += sizeof(unsigned int) * shape.mesh.indices.size();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}	  
	//============================================================

	//===================== TEXTURES =============================
	// get name of texture image from loaded obj
	int materialId = shapes[0].mesh.material_ids[0];
	std::string textureFilename = materials[materialId].diffuse_texname;
	std::string texturePath = objBase + textureFilename;

	// load the texture image
	int imgWidth;
	int imgHeight;
	int nColorDepth;
	unsigned char* pixels = stbi_load(texturePath.c_str(), &imgWidth, &imgHeight, &nColorDepth, 0);

	if (pixels == NULL)
	{
		fprintf(stderr, "Failed to load texture: %s\n", texturePath.c_str());
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

	//======================== LIGHTS ============================
	glm::vec3 light = glm::vec3(4, 4, 4);
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
		GLuint iModelLoc = glGetUniformLocation(*shaderId, "iModel");
		GLuint iViewLoc = glGetUniformLocation(*shaderId, "iView");
		GLuint iTextureSamplerLoc = glGetUniformLocation(*shaderId, "iTextureSampler");
		GLuint iLightPosition_worldspaceLoc = glGetUniformLocation(*shaderId, "iLightPosition_worldspace");

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

		// activate texture channel 0 and pass it to shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		if (iTextureSamplerLoc != -1)
		{
			glUniform1i(iTextureSamplerLoc, 0);
		}

		// pass light position uniform to shader
		if (iLightPosition_worldspaceLoc != -1) 
		{
			glUniform3f(iLightPosition_worldspaceLoc, light.x, light.y, light.z);
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
		// attach variables for each shape
		size_t vertex_buffer_offset = 0;
		size_t texCoords_buffer_offset = 0;
		size_t normals_buffer_offset = 0;
		size_t indices_buffer_offset = 0;
		for each(tinyobj::shape_t shape in shapes)
		{
			// Attach position buffer as attribute 0
			if (positionVBO != 0)
			{
				glBindVertexArray(meshVAO);

				// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
				// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
				glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)vertex_buffer_offset);
				//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
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
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)texCoords_buffer_offset);
				//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
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
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)normals_buffer_offset);
				//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
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

			// Draw the vertices of the current shape at the current offset
			glDrawElements(
				GL_TRIANGLES,
				shape.mesh.indices.size(),
				GL_UNSIGNED_INT,
				(void*)indices_buffer_offset
			);

			// increment offsets
			vertex_buffer_offset += sizeof(float)* shape.mesh.positions.size();
			texCoords_buffer_offset += sizeof(float)* shape.mesh.texcoords.size();
			normals_buffer_offset += sizeof(float)* shape.mesh.normals.size();
			indices_buffer_offset += sizeof(float)* shape.mesh.indices.size();

			// disable the attribute after drawing
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}		

		// Draw the vertices
		/*
		glDrawElements(
		GL_TRIANGLES,      // mode
		shapes[0].mesh.indices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
		);
		*/

		

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