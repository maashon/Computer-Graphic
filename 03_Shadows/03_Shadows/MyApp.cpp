#include "MyApp.h"

#include <math.h>
#include <vector>
#include <array>
#include <list>
#include <tuple>

#include "imgui/imgui.h"

#include "Includes/ObjParser_OGL3.h"

CMyApp::CMyApp(void){}


CMyApp::~CMyApp(void){}

bool CMyApp::Init()
{
	glClearColor(0.2f, 0.4f, 0.7f, 1);	// Clear color will be white
	glEnable(GL_CULL_FACE);				// Drop faces looking backwards
	glEnable(GL_DEPTH_TEST);			// Enable depth test

	m_program.Init({	//Shader for drawing geometries
		{ GL_VERTEX_SHADER, "Shaders/myVert.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/myFrag.frag" }
	},{
		{ 0, "vs_in_pos" },		// VAO index 0 will be vs_in_pos
		{ 1, "vs_in_normal" },	// VAO index 1 will be vs_in_normal
		{ 2, "vs_out_tex0" },	// VAO index 2 will be vs_in_tex0
	});
		
	m_programPostprocess.Init({ // Shadow shader
		{ GL_VERTEX_SHADER,		"Shaders/shadow_map.vert" },
		{ GL_FRAGMENT_SHADER,	"Shaders/shadow_map.frag" }
	},{
		{ 0, "vs_in_pos" }		// Only Position is needed here
	});

	// Creating VBOs for 
	ArrayBuffer positions(std::vector<glm::vec3>{
		glm::vec3(-20, 0, -20), glm::vec3(-20, 0, 20), glm::vec3(20, 0, -20), glm::vec3(20, 0, 20)
	});
	ArrayBuffer normals(std::vector<glm::vec3>{
		glm::vec3(0, 1, 0),     glm::vec3(0, 1, 0),    glm::vec3(0, 1, 0),    glm::vec3(0, 1, 0)
	});
	ArrayBuffer texture(std::vector<glm::vec2>{
		glm::vec2(0, 0),        glm::vec2(0, 1),       glm::vec2(1, 0),       glm::vec2(1, 1)
	});
	IndexBuffer indices(std::vector<uint16_t>{0, 1, 2,  2, 1, 3});

	m_vao.Bind();
	m_vao.AddAttribute(CreateAttribute<  0, glm::vec3, 0, sizeof(glm::vec3)>, positions);
	m_vao.AddAttribute(CreateAttribute<  1, glm::vec3, 0, sizeof(glm::vec3)>, normals);
	m_vao.AddAttribute(CreateAttribute<  2, glm::vec2, 0, sizeof(glm::vec2)>, texture);
	m_vao.SetIndices(indices);
	m_vao.Unbind();

	m_textureMetal.FromFile("Assets/texture.png"); // Load a texture

	m_mesh = ObjParser::parse("Assets/Suzanne.obj"); // Load the monkey mesh

	m_camera.SetProj(45.0f, m_width / m_height, 0.01f, 1000.0f); //Set the camer projection (fow, aspect ratio, near and far clipping distance)

	CreateFrameBuffer(1024, 1024);

	return true;
}

void CMyApp::Clean()
{
	if (m_frameBufferCreated)	{
		glDeleteTextures(1, &m_shadow_texture);
		glDeleteFramebuffers(1, &m_frameBuffer);
	}
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;
	m_camera.Update(delta_time);

	last_time = SDL_GetTicks();
}

void CMyApp::DrawScene(const glm::mat4 &viewProj, ProgramObject& program, bool shadowProgram = false)
{
	program.Use();

	if (!shadowProgram) {
		program.SetTexture("textureShadow", 1, m_shadow_texture); // depth values
		program.SetTexture("texImage", 0, m_textureMetal);
		program.SetUniform("shadowVP", m_light_mvp); //so we can read the shadow map
		program.SetUniform("toLight", -m_light_dir);
	}

	// Drawing the plane underneath

	program.SetUniform("MVP", viewProj * glm::mat4(1));
	if (!shadowProgram) {
		program.SetUniform("world",   glm::mat4(1));
		program.SetUniform("worldIT", glm::mat4(1));
		program.SetUniform("Kd", glm::vec4(0.1, 0.9, 0.3, 1));
	}

	m_vao.Bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	//Draws exactly the same but uses index buffer:
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	m_vao.Unbind();

	// Suzanne wall

	float t = SDL_GetTicks() / 1000.f;
	for (int i = -1; i <= 1; ++i)
		for (int j = -1; j <= 1; ++j)
		{
			glm::mat4 suzanneWorld = glm::translate(glm::vec3(4 * i, 4 * (j + 1), sinf(t * 2 * M_PI * i * j)));
			program.SetUniform("MVP", viewProj * suzanneWorld);
			if (!shadowProgram) {
				program.SetUniform("world", suzanneWorld);
				program.SetUniform("worldIT", glm::transpose(glm::inverse(suzanneWorld)));	// <- how could we simplify this?
				program.SetUniform("Kd", glm::vec4(1, 0.3, 0.3, 1));
			}
			m_mesh->draw();
		}
	program.Unuse();
}

void CMyApp::Render()
	
{
	// 1.
	// Draw scene to shadow map
	static glm::ivec2 wh = glm::ivec2(1024);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);	// FBO that has the shadow map attached to it
	glViewport(0, 0, wh.x, wh.y);						// Set which pixels to render to within this fbo.
	glClear(GL_DEPTH_BUFFER_BIT);						// Clear depth values
	glm::mat4 light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
	glm::mat4 light_view = glm::lookAt<float>(glm::vec3(0,0,0), m_light_dir, glm::vec3(0, 1, 0));
	m_light_mvp = light_proj * light_view; // This matrix will tell us how to read the distances in the shadow map
	DrawScene(m_light_mvp, m_programPostprocess, true);

	// 2.
	// Draw mesh to screen

	glBindFramebuffer(GL_FRAMEBUFFER, 0);				// default framebuffer (the backbuffer)
	glViewport(0, 0, m_width, m_height);				// We need to set the render area back
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clearing the default fbo
	DrawScene(m_camera.GetViewProj(),m_program);

	// 3.
	// User Interface

	ImGui::ShowTestWindow(); // Demo of all ImGui commands. See its implementation for details.
	// It's worth browsing imgui.h, as well as reading the FAQ at the beginning of imgui.cpp.
	// There is no regular documentation, but the things mentioned above should be sufficient.

	ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Test window");
	{
		//ImGui::SliderFloat("t", &m_filterWeight, 0, 1);
		ImGui::SliderFloat3("light_dir", &m_light_dir.x, -1.f, 1.f);
		m_light_dir = glm::normalize(m_light_dir); // This needs to remain a normalized direction
		ImGui::Image((ImTextureID)m_shadow_texture, ImVec2(256, 256));
		if (ImGui::SliderInt("Resolution x", &wh.x, 4, 8096)) {

			CreateFrameBuffer(wh.x, wh.y);
		}
	}
	ImGui::End();
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// _w and _h are the width and height of the window's size
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );

	m_camera.Resize(_w, _h);
	m_width = _w;
	m_height = _h;

	//CreateFrameBuffer(_w, _h); // This FBO has a fixed size!
}

inline void setTexture2DParameters(GLenum magfilter = GL_LINEAR, GLenum minfilter = GL_LINEAR, GLenum wrap_s = GL_CLAMP_TO_EDGE, GLenum wrap_t = GL_CLAMP_TO_EDGE)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}

void CMyApp::CreateFrameBuffer(int width, int height)
{
	// Clear if the function is not being called for the first time
	if (m_frameBufferCreated) {
		glDeleteTextures(1, &m_shadow_texture);
		glDeleteFramebuffers(1, &m_frameBuffer);
	}

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	glGenTextures(1, &m_shadow_texture); // Create texture holding the depth components
	glBindTexture(GL_TEXTURE_2D, m_shadow_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST); //so its shorter

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadow_texture, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	glDrawBuffer(GL_NONE); // No need for any color output!
	   		
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); // -- Completeness check
	if (status != GL_FRAMEBUFFER_COMPLETE)	{
		std::cout << "Incomplete framebuffer (";
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";		break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";					break;
		}
		std::cout << ")" << std::endl;
		char ch;std::cin >> ch;
		exit(1);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	// -- Unbind framebuffer
	m_frameBufferCreated = true;
}