#pragma once

// C++ includes
#include <memory>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "Includes/ProgramObject.h"
#include "Includes/BufferObject.h"
#include "Includes/VertexArrayObject.h"
#include "Includes/TextureObject.h"

#include "Includes/Mesh_OGL3.h"
#include "Includes/gCamera.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

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
	
	// This function defines the scene now
	void DrawScene(const glm::mat4& viewProj, ProgramObject& program, bool shadowProgram);
	
	// FBO creating function
	void CreateFrameBuffer(int width, int height);

	// variables for shaders
	ProgramObject		m_program;				// basic program for shaders
	ProgramObject		m_programPostprocess;	// posprocess shaderek program

	Texture2D			m_textureMetal;
	VertexArrayObject	m_vao;
	
	std::unique_ptr<Mesh>	m_mesh;

	gCamera				m_camera;
	int	m_width = 640, m_height = 480;

	glm::mat4 m_light_mvp;
	glm::vec3 m_light_dir = glm::normalize(glm::vec3(0,-1,-1));
		
	// stuffs for the FBO
	bool m_frameBufferCreated{ false };
	GLuint m_shadow_texture;
	GLuint m_frameBuffer;
};

