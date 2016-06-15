#include <SDL.h>

#include "opengl.h"

#include <stdio.h>

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
    SDL_Window* window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

	//======================== VAO ===============================
	// generate VAO reference
	GLuint vertexVAO;
	glGenVertexArrays(1, &vertexVAO);

	// make the VAO active
	glBindVertexArray(vertexVAO);
	//============================================================	

	//===================== VBO ==================================
	// define vertex array
	static const GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

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

		
        // Set the color to clear with
        glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

		//================ SET VAO AND DRAW ===========================
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

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

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