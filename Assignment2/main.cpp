#include <stdio.h>

#include <SDL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

#include "opengl.h"
#include "shaderset.h"

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
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

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

	//=================== MATRICES ==============================
	// Projection matrix
	glm::mat4 Projection = glm::perspective(
		glm::radians(45.0f),						// field of view (degrees)
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, // aspect ratio
		0.1f,										// near clip plane
		100.0f										// far clip plane
	);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 ModelViewProjection = Projection * View * Model; // Remember, matrix multiplication is the other way around
	//============================================================

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

	// Generate VBO reference
	GLuint vertexbuffer;	
	glGenBuffers(1, &vertexbuffer);

	// make the VBO active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	// upload vertices to buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//============================================================

    // Begin main loop
    while (1)
    {
        // Handle all events since the last update
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                goto quit;
            }
        }
		
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
        glClear(GL_COLOR_BUFFER_BIT);

		//================ UPDATE VAO AND DRAW ===========================
		// Note: glVertexAttribPointer sets the current GL_ARRAY_BUFFER_BINDING as the source of data for this attribute
		// That's why we bind a GL_ARRAY_BUFFER before calling glVertexAttribPointer then unbind right after (to clean things up).
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

		// specify how the attribute is formatted and retrieved
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// Enable the attribute (they are disabled by default -- this is very easy to forget!!)
		glEnableVertexAttribArray(0);

		// Draw the vertices
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

		// disable the attribute after drawing
		glDisableVertexAttribArray(0);
		//============================================================

        // SDL docs: "On Mac OS X make sure you bind 0 to the draw framebuffer before swapping the window, otherwise nothing will happen."
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        // Display the frame of rendering to the window
        SDL_GL_SwapWindow(window);		
    }

quit:
    SDL_Quit();
    return 0;
}