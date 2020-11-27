#include "GL/glew.h"
#include <stdio.h>
#include <stdlib.h>

static char * ReadShaderSource(const char *file_name)
// Read shader source codes from a file
// file_name: (in) Pointer to the string containing the file name
// Return value: Pointer to the string containing the source codes
// Memory is allocated for the returned string
{
	// Open the file
	FILE *fp=fopen(file_name, "rb");

	// Check if the file is successfully opened
	if (fp==NULL) return NULL;

	// Move the file pointer to the end of the file
	fseek(fp, 0, SEEK_END);

	// Get the file size as current position of the file pointer
	long size=ftell(fp);

	// Allocate memory for the source code string
	char *str=new char [size+1];

	// Move the file pointer to the beginning of the file
	fseek(fp, 0, SEEK_SET);

	// Read the entire file content to the source code string
	fread(str, 1, size, fp);

	// Close the file
	fclose(fp);

	// Add a terminal null character to the end of the source code string
	str[size]=0;

	// Return the string pointer
	return str;
}

GLuint InitShader(
	const char *vShaderFile, 
	const char *fShaderFile)
// Initialize a shader program
// vShaderFile: (in) Pointer to the file containing the vertex shader source codes
// fShaderFile: (in) Pointer to the file containing the fragment shader source codes
// Return value: The name (index) of the shader program object
{
	// Shader information structure
	struct {
		const char *file_name; // Shader source code file name
		GLenum type; // Shader type
	} shaders[2]= // Shader information array
	{
		{vShaderFile, GL_VERTEX_SHADER},
		{fShaderFile, GL_FRAGMENT_SHADER}
	};

	// Create the program object
	GLuint prog=glCreateProgram();

	// Loop through all shaders
	for (int i=0; i<2; i++)
	{
		// Read the shader source codes from the file
		char *src=ReadShaderSource(shaders[i].file_name);

		// Check if the shader source codes is successfully read
		if (src==NULL)
		{
			printf("Unable to read source codes from %s.\n",
				shaders[i].file_name);
			while (getchar()!='\n') ;
			exit(0);
		}
		
		// Create the shader
		GLuint shader=glCreateShader(shaders[i].type);

		// Set the shader source codes
		glShaderSource(shader, 1, (const char **)&src, NULL);

		// Compile the shader
		glCompileShader(shader);

		// Check if the shader is successfully compiled
		GLint compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (!compile_status)
		{
			// The shader is not successfully compiled
			// Display an error message in the console
			printf("Failed to compile shader %s.\n",
				shaders[i].file_name);

			// Get the shader information log length
			GLint info_log_len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, 
				&info_log_len);

			// Allocate memory for the shader information log
			char *info_log=new char [info_log_len];

			// Get the shader information log
			glGetShaderInfoLog(shader, 
				info_log_len, NULL, info_log);

			// Display the shader information log in the console
			printf("%s\n", info_log);

			// Free memory for the shader information log
			delete [] info_log;

			while (getchar()!='\n') ;
			exit(0);
		}

		// Attach the shader to the program
		glAttachShader(prog, shader);

		// Free memory for the shader source codes
		delete [] src;
	}

	// Link the program
	glLinkProgram(prog);

	// Check if the program is successfully linked
	GLint link_status;
	glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
	if (!link_status)
	{
		// The program is not successfully linked
		// Display an error message in the console
		printf("Failed to link program.\n");

		// Get the program information log length
		GLint info_log_len;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, 
			&info_log_len);

		// Allocate memory for the program information log
		char *info_log=new char [info_log_len];

		// Get the program information log
		glGetProgramInfoLog(prog, 
			info_log_len, NULL, info_log);

		// Display the program information log in the console
		printf("%s\n", info_log);

		// Free memory for the program information log
		delete [] info_log;

		while (getchar()!='\n') ;
		exit(0);
	}

	// Set the program as current active program
	glUseProgram(prog);

	// Return the program name (index)
	return prog;
}

GLuint InitShader(
	const char* vShaderFile,
	const char* tcShaderFile,
	const char* teShaderFile,
	const char* fShaderFile)
	// Initialize a shader program
	// vShaderFile: (in) Pointer to the file containing the vertex shader source codes
	// fShaderFile: (in) Pointer to the file containing the fragment shader source codes
	// Return value: The name (index) of the shader program object
{
	// Shader information structure
	struct {
		const char* file_name; // Shader source code file name
		GLenum type; // Shader type
	} shaders[4] = // Shader information array
	{
		{vShaderFile, GL_VERTEX_SHADER},
		{tcShaderFile, GL_TESS_CONTROL_SHADER},
		{teShaderFile, GL_TESS_EVALUATION_SHADER},
		{fShaderFile, GL_FRAGMENT_SHADER}
	};

	// Create the program object
	GLuint prog = glCreateProgram();

	// Loop through all shaders
	for (int i = 0; i < 4; i++)
	{
		// Read the shader source codes from the file
		char* src = ReadShaderSource(shaders[i].file_name);

		// Check if the shader source codes is successfully read
		if (src == NULL)
		{
			printf("Unable to read source codes from %s.\n",
				shaders[i].file_name);
			while (getchar() != '\n');
			exit(0);
		}

		// Create the shader
		GLuint shader = glCreateShader(shaders[i].type);

		// Set the shader source codes
		glShaderSource(shader, 1, (const char**)&src, NULL);

		// Compile the shader
		glCompileShader(shader);

		// Check if the shader is successfully compiled
		GLint compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (!compile_status)
		{
			// The shader is not successfully compiled
			// Display an error message in the console
			printf("Failed to compile shader %s.\n",
				shaders[i].file_name);

			// Get the shader information log length
			GLint info_log_len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH,
				&info_log_len);

			// Allocate memory for the shader information log
			char* info_log = new char[info_log_len];

			// Get the shader information log
			glGetShaderInfoLog(shader,
				info_log_len, NULL, info_log);

			// Display the shader information log in the console
			printf("%s\n", info_log);

			// Free memory for the shader information log
			delete[] info_log;

			while (getchar() != '\n');
			exit(0);
		}

		// Attach the shader to the program
		glAttachShader(prog, shader);

		// Free memory for the shader source codes
		delete[] src;
	}

	// Link the program
	glLinkProgram(prog);

	// Check if the program is successfully linked
	GLint link_status;
	glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
	if (!link_status)
	{
		// The program is not successfully linked
		// Display an error message in the console
		printf("Failed to link program.\n");

		// Get the program information log length
		GLint info_log_len;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH,
			&info_log_len);

		// Allocate memory for the program information log
		char* info_log = new char[info_log_len];

		// Get the program information log
		glGetProgramInfoLog(prog,
			info_log_len, NULL, info_log);

		// Display the program information log in the console
		printf("%s\n", info_log);

		// Free memory for the program information log
		delete[] info_log;

		while (getchar() != '\n');
		exit(0);
	}

	// Set the program as current active program
	glUseProgram(prog);

	// Return the program name (index)
	return prog;
}