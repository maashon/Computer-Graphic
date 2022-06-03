// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// STL 
#include <iostream>
#include <sstream>

// In this project
#include "GLDebugMessageCallback.h"
#include "MyApp.h"
// this main function is not a real main as its a defined function in sdl
int main( int argc, char* args[] )
{
	// Assign a lambda function to be called when the program exits. This will
	// make the application wait for a keypress before closing the console
	// window, giving us an opportunity to read any logs printed to the console.
	atexit([] {
			std::cout << "Press a key to exit the application..." << std::endl;
			std::cin.get();
		});

	//
	// Step 1: Initialize SDL
	//

	// Try initializing the graphical subsystem only (SDL_INIT_VIDEO).
	if ( SDL_Init( SDL_INIT_VIDEO ) == -1 )
	{
		// If it fails, then log the error and exit.
		std::cout << "[SDL initialization] Error during the SDL initialization: " << SDL_GetError() << std::endl;
		return 1;
	}
			
	//
	// Step 2: Configure OpenGL, create a window, then start OpenGL.
	//

	// 2a: Configuring OpenGL. This MUST happen before creating the window.

	// We may now set the desired OpenGL version - without explicitly doing so,
	// OpenGL will default to the highest version available.
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef _DEBUG   
	// Put OpenGL into debug mode if we are building a debug version.
	// This is necessary for enabling debug callbacks later.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	// Set the minimum number of bits per pixel for the different channels (red,
	// green, blue and transparency).
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,         32);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);
	// Enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,		1);
	// Set the minimum number of bits per pixel for the depth buffer as well.
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          24);

	// Anti alisaing - if needed
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

	// Let's create the window
	SDL_Window *win = nullptr;
	win = SDL_CreateWindow(
		"Hello SDL&OpenGL!",		// window title
		100,						// the X position of the window's top-left corner
		100,						// the Y position of the window's top-left corner
		800,						// width
		600,						// height
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE	// flags / display properties
	);


	// If creating the window fails log the error and quit.
	if (win == nullptr)
	{
		std::cout << "[Window creation] Error during the creation of an SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//
	// Step 3: Create the OpenGL context - this is what we will use for drawing
	//

	SDL_GLContext	context	= SDL_GL_CreateContext(win);
	if (context == nullptr)
	{
		std::cout << "[OGL context creation] Error during the creation of the OGL context: " << SDL_GetError() << std::endl;
		return 1;
	}	

	// Wait for the vsync.
	SDL_GL_SetSwapInterval(1);

	// Initialize GLEW
	GLenum error = glewInit();
	if ( error != GLEW_OK )
	{
		std::cout << "[GLEW] Error during the initialization of glew:" << std::endl;
		return 1;
	}

	// Query the OpenGL version.
	int glVersion[2] = {-1, -1}; 
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); 
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); 
	std::cout << "Running OpenGL " << glVersion[0] << "." << glVersion[1] << std::endl;

	if ( glVersion[0] == -1 && glVersion[1] == -1 )
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow( win );

		std::cout << "[OGL context creation] Error during the inialization of the OGL context! Maybe one of the SDL_GL_SetAttribute(...) calls is erroneous." << std::endl;

		return 1;
	}

	std::stringstream window_title;
	window_title << "OpenGL " << glVersion[0] << "." << glVersion[1];
	SDL_SetWindowTitle(win, window_title.str().c_str());

	// Enable and configure debug callbacks if we are inside a debug context.
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(GLDebugMessageCallback, nullptr);
	}

	//
	// Step 4: Start the event loop
	// 

	// Signals whether the event loop should stop
	bool quit = false;
	
	// Store the event waiting to be processed.
	SDL_Event ev;
	
	// Let's instantiate and initialize our application.
	CMyApp app;
	if (!app.Init())
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		std::cout << "[app.Init] Error during the initialization of the application!" << std::endl;
		return 1;
	}

	// "Infinite" loop that iterates over each frame until we quit
	while (!quit)
	{
		// Iterate over each event that occured during the last frame
		while ( SDL_PollEvent(&ev) )
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if ( ev.key.keysym.sym == SDLK_ESCAPE )
					quit = true;
				app.KeyboardDown(ev.key);
				break;
			case SDL_KEYUP:
				app.KeyboardUp(ev.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				app.MouseDown(ev.button);
				break;
			case SDL_MOUSEBUTTONUP:
				app.MouseUp(ev.button);
				break;
			case SDL_MOUSEWHEEL:
				app.MouseWheel(ev.wheel);
				break;
			case SDL_MOUSEMOTION:
				app.MouseMove(ev.motion);
				break;
			case SDL_WINDOWEVENT:
				if ( ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
				{
					app.Resize(ev.window.data1, ev.window.data2);
				}
				break;
			}
		} // end of event loop

		app.Update(); // Phyics update
		app.Render(); // Anything that needs to be drawn

		SDL_GL_SwapWindow(win); // Draw this is where the waiting happens. if we want to measure the fps, we should set the
		// wait of vsync to 0 as we want to measure the fps not tyhe vsync time.
		// all the above commandas run extremely fast. the wait happens here as we set
		// SDL_GL_SetSwapInterval to 1. if we 
	}


	//
	// Step 5: Quitting
	// 

	// Let's clean up after ourselves.
	app.Clean();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow( win );

	return 0;
}