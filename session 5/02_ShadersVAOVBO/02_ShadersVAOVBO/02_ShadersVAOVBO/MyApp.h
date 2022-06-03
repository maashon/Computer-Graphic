#pragma once

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>

class CMyApp
{
public:
	CMyApp() = default;
	~CMyApp() = default;

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	// Variables for the shaders:
	GLuint m_programID = 0; // Shader program

	// Variables for OpenGL:
	GLuint m_vaoID = 0; // Vertex Array Object resource ID
	GLuint m_vboID = 0; // Vertex Buffer Object resource ID

	// Variables for texture(s):
	GLuint m_loc_tex = 0; 				// Uniform location for the sampler2D
	GLuint m_loadedTextureID = 0; 		// Texture resource ID for the loaded texture
	GLuint m_generatedTextureID = 0;	// Texture resource ID for the generated texture

	struct Vertex	
	{
		glm::vec3 p; // position
		glm::vec3 c; // color
		glm::vec2 t; // texture coordinates
	};
};

