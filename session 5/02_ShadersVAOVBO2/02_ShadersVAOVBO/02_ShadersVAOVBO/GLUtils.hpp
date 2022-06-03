#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <GL/glew.h>

#include <SDL_image.h>

/* 

Based on http://www.opengl-tutorial.org/

*/
GLuint loadShader(GLenum _shaderType, const char* _fileName)
{
	// Let's create the shader object
	GLuint loadedShader = glCreateShader(_shaderType);

	// Upon failure, print an error message and return with 0.
	// Note that glCreateShader() will never return with 0, so this won't
	// cause any problems.
	if (loadedShader == 0)
	{
		std::cerr << "[glCreateShader] Error during the initialization of shader: " << _fileName << "!\n";
		return 0;
	}

	// Loading a shader from disk.
	std::string shaderCode = "";

	// Open '_fileName'
	std::ifstream shaderStream(_fileName);

	if (!shaderStream.is_open())
	{
		std::cerr << "[std::ifstream] Error during the reading of " << _fileName << " shaderfile's source!\n";
		return 0;
	}

	// Load the contents of the file into the 'shaderCode' variable.
	std::string line = "";
	while (std::getline(shaderStream, line))
	{
		shaderCode += line + "\n";
	}

	shaderStream.close();

	// Assign the loaded source code to the shader object.
	const char* sourcePointer = shaderCode.c_str();
	glShaderSource(loadedShader, 1, &sourcePointer, nullptr);

	// Let's compile the shader.
	glCompileShader(loadedShader);

	// Check whether the compilation was successful.
	GLint result = GL_FALSE;
	int infoLogLength;

	// For this, retrieve the status.
	glGetShaderiv(loadedShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(loadedShader, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (GL_FALSE == result)
	{
		// Get and print the error message.
		std::vector<char> VertexShaderErrorMessage(infoLogLength);
		glGetShaderInfoLog(loadedShader, infoLogLength, nullptr, &VertexShaderErrorMessage[0]);

		std::cerr << "[glCompileShader] Shader compilation error in " << _fileName << ":\n" << &VertexShaderErrorMessage[0] << std::endl;
	}

	return loadedShader;
}

int invert_image(int pitch, int height, void* image_pixels)
{
	int index;
	void* temp_row;
	int height_div_2;

	temp_row = (void *)malloc(pitch);
	if (NULL == temp_row)
	{
		SDL_SetError("Not enough memory for image inversion");
		return -1;
	}
	// if height is odd, don't need to swap middle row
	height_div_2 = (int)(height * .5);
	for (index = 0; index < height_div_2; index++) {
		// uses string.h
		memcpy((Uint8 *)temp_row,
			(Uint8 *)(image_pixels)+
			pitch * index,
			pitch);

		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * index,
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			pitch);
		memcpy(
			(Uint8 *)(image_pixels)+
			pitch * (height - index - 1),
			temp_row,
			pitch);
	}
	free(temp_row);
	return 0;
}

int SDL_InvertSurface(SDL_Surface* image)
{
	if (NULL == image)
	{
		SDL_SetError("Surface is NULL");
		return -1;
	}

	return invert_image(image->pitch, image->h, image->pixels);
}

GLuint TextureFromFile(const char* filename)
{

	// Load the image
	SDL_Surface* loaded_img = IMG_Load(filename);
	if (loaded_img == nullptr)
	{
		std::cout << "[TextureFromFile] Error during the loading of image '" << filename << "'" << std::endl;
		return 0;
	}

	// SDL stores the colors in Uint32, hence we need to account for the byte
	// order here.
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#else
	Uint32 format = SDL_PIXELFORMAT_RGBA8888;
#endif

	// Convert the image format to 32bit RGBA if it wasn't that already.
	SDL_Surface* formattedSurf = SDL_ConvertSurfaceFormat(loaded_img, format, 0);
	if (formattedSurf == nullptr)
	{
		std::cout << "[TextureFromFile] Error during the processing of the image: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(loaded_img);
		return 0;
	}

	// While (0,0) in SDL means top-left, in OpenGL it means bottom-left, so we
	// need to convert it from one to another.
	if (SDL_InvertSurface(formattedSurf) == -1) {
		std::cout << "[TextureFromFile] Error during the processing of the image: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(formattedSurf);
		SDL_FreeSurface(loaded_img);
		return 0;
	}

	// Generate the OpenGL texture resource
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formattedSurf->w, formattedSurf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurf->pixels);

	// Generate Mipmap
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Free the SDL_Surfaces
	SDL_FreeSurface(formattedSurf);
	SDL_FreeSurface(loaded_img);

	return tex;
}