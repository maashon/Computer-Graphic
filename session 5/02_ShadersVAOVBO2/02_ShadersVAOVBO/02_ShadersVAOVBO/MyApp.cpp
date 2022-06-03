#include "MyApp.h"
#include "GLUtils.hpp"

#include <cstddef>
#include <cmath>
#include <ctime>

bool CMyApp::Init()
{
	// Set a bluish clear color. glClear() will use this for clearing the color
	// buffer.
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	// Enable discarding the back-facing faces.
	glEnable(GL_CULL_FACE);

	// Enable depth testing. (for overlapping geometry)
	glEnable(GL_DEPTH_TEST);

	//
	// Creating the geometry
	//

	// [position] x [color] x [texture coordinates]
	std::vector<Vertex> vert =
	{ 
		{glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec2(0, 0)},
		{glm::vec3( 1, -1, 0), glm::vec3(0, 1, 0), glm::vec2(1, 0)},
		{glm::vec3(-1,  1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1)},
		{glm::vec3( 1,  1, 0), glm::vec3(1, 1, 1), glm::vec2(1, 1)},
	};

	// Generate 1 Vertex Array Object (VAO)
	glGenVertexArrays(1, &m_vaoID);
	glBindVertexArray(m_vaoID); // Make the freshly generated VAO active (bind it)
	
	// Generate 1 Vertex Buffer Object (VAO)
	glGenBuffers(1, &m_vboID); 
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID); // Make it active by binding it

	// Transfer data to the currently active VBO.
	glBufferData(
		GL_ARRAY_BUFFER,	// set the target to the active VBO
		sizeof(Vertex)*vert.size(),		// number of BYTES
		vert.data(),		// pointer to the data
		GL_STATIC_DRAW		// we do not intend to modify the data later, but we will use the buffer in a LOT of draw calls
	);

	// Configure the VAO so that the first attribute (position) in the VBO will
	// span over 3 floats. Setting the stride to sizeof(Vertex) means that
	// OpenGL will advance the pointer this much to find the position of the
	// next vertex.
	glEnableVertexAttribArray(0); // vertex position
	glVertexAttribPointer(
		(GLuint)0,					// setting the 0th attribute of the VB
		3,							// number of componens (vec3 -> 3)
		GL_FLOAT,					// data type of the components
		GL_FALSE,					// normalize or not
		sizeof(Vertex),				// stride (0 would mean tightly packed)
		(void*)offsetof(Vertex, p)	// = 0 the offset of the position attribute in the Vertex struct
	);

	// Note: offsetoff(STRUCT, MEMBER) will return a member's offset in a
	//  struct, pretty useful for glVertexAttribPointer()

	// The second attribute in the VBO is color. In the VBO it starts right
	// after the position data which we hence need to skip by specifying
	// 'sizeof(glm::vec3)' or 'offsetof(Vertex, c)' in the last argument.
	glEnableVertexAttribArray(1); // vertex color
	glVertexAttribPointer(
		(GLuint)1,
		3, 
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, c) // sizeof(glm::vec3)
	);

	// And we store texture coordinates in the third attribute.
	glEnableVertexAttribArray(2); // texture coordinates
	glVertexAttribPointer(
		(GLuint)2,
		2, 
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, t) // sizeof(glm::vec3) + sizeof(glm::vec3)
	);

	// We are done with these, unbind them
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//
	// Loading the shaders
	//
	GLuint vs_ID = loadShader(GL_VERTEX_SHADER,		"myVert.vert");
	GLuint fs_ID = loadShader(GL_FRAGMENT_SHADER,	"myFrag.frag");

	// Create a program that will hold the shaders
	m_programID = glCreateProgram();

	// Attach the shaders to the program.
	glAttachShader(m_programID, vs_ID);
	glAttachShader(m_programID, fs_ID);

	// Associate a vertex attribute index from the VAO with a named attribute
	// variable in the shader.
	glBindAttribLocation( m_programID, 0, "vs_in_pos");
	glBindAttribLocation( m_programID, 1, "vs_in_col");
	glBindAttribLocation( m_programID, 2, "vs_in_tex0");

	// Link the shaders program.
	//  (this will connect the in-out variables between shaders, etc...)
	glLinkProgram(m_programID);

	// Checking whether the linking was successful
	GLint infoLogLength = 0, result = 0;

	glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (GL_FALSE == result || infoLogLength != 0)
	{
		std::vector<char> VertexShaderErrorMessage(infoLogLength);
		glGetProgramInfoLog(m_programID, infoLogLength, nullptr, VertexShaderErrorMessage.data());

		std::cerr << "[glLinkProgram] Shader linking error:\n" << &VertexShaderErrorMessage[0] << std::endl;
	}

	// We can safely delete these, as we do not need them anymore.
	glDeleteShader( vs_ID );
	glDeleteShader( fs_ID );

	//
	// Loading a texture
	//

	// Load the texture from a file
	m_loadedTextureID = TextureFromFile("wood.jpg");

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, m_loadedTextureID);

	// Set the sampling parameters
	// bilinear filtering for magnification (default value)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// trilinear filtering from the mipmaps while minifying (default value)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// How should sampling from outside of the texture behave?
	// GL_CLAMP_TO_EDGE will clamp the texture coordinates between 0 and 1
	// meaning that the texture's edges will "extend to infinity".
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // vertically
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // horizontally

	// Get the sampler2D's uniform location, and save it for later use.
	m_loc_tex = glGetUniformLocation(m_programID, "texImage");

	//
	// Generating a texture
	// 

	const int W = 128, H = 128;
	unsigned char tex[W][H][3];

	// Optional: seed the random
	srand(time(NULL));

	// Generate a random texture
	for (int i = 0; i < W; ++i) {
		for (int j = 0; j < H; ++j) {
			tex[i][j][0] = rand() % 256;
			tex[i][j][1] = rand() % 256;
			tex[i][j][2] = rand() % 256;
		}
	}

	// Generate the OpenGL texture resource
	glGenTextures(1, &m_generatedTextureID);

	// Activate the generated texture
	glBindTexture(GL_TEXTURE_2D, m_generatedTextureID);

	// Fill it up with data
	glTexImage2D(GL_TEXTURE_2D,	// the binding point that holds the texture
		0,						// level-of-detail
		GL_RGB,					// texture's internal format (GPU side)
		W, H,					// width, height
		0,						// must be 0 ( https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml )
		GL_RGB,					// source (CPU side) format
		GL_UNSIGNED_BYTE,		// data type of the pixel data (CPU side)
		tex);					// pointer to the data

	glGenerateMipmap(GL_TEXTURE_2D);

	// Set sampling parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Finally unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &m_loadedTextureID);

	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);

	glDeleteProgram( m_programID );
}

void CMyApp::Update()
{
}


void CMyApp::Render()
{
	// Clear the framebuffer (GL_COLOR_BUFFER_BIT) and the depth buffer (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// in opengl the back buffer is between [-1,1] and this command clears the color buffer
	//to 0 , and the depth buffer to 1

	// Enable the shader program.
	glUseProgram( m_programID );

	// Bind the VAO (no need to call glBindBuffer(), since the VBO is already
	// attached to the VAO)
	glBindVertexArray(m_vaoID);

	// Binding the texture
	// Let's activate the 0th texture sampler => from now on any gl*Texture*()
	// calls will refer to this sampler. This is important when we are using
	// multiple textures simultaneously. By default the 0th sampler is active,
	// hence we could even omit this call.
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, m_loadedTextureID);
	// glBindTexture(GL_TEXTURE_2D, m_generatedTextureID);

	// Set the "texImage" sampler2D variable in the shader to point to the 0th
	// texture sampler.
	glUniform1i(m_loc_tex, 0);

	// Drawing
	// A VAO and a shader program must already be bound (using: glUseProgram() and glBindVertexArray())
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Unbind the VAO.
	glBindVertexArray(0);

	// Disable the shader program.
	glUseProgram( 0 );
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
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

// Resize event containing the new width (_w) and height (_h) of the window.
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );
}