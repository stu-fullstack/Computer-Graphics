#include "ImageLib.h"
#include "FreeImage.h"
#include "GL/glew.h"

unsigned int LoadTexture2DFromFile(const char *file_name)
// Create a 2D texture object and load its image from an image file
// file_name: (in) Image file name string
// Return value: Name (index) of the texture object
{
	// Get the image file format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // Image Format
	// Check the file signature and deduce its format
	fif = FreeImage_GetFileType(file_name, 0);
	// If unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(file_name);
	if(fif == FIF_UNKNOWN)
		return 0;

	// Load the image from file
	FIBITMAP* img = FreeImage_Load(fif, file_name);
	if (img==NULL) return 0;

	// Get the image information
	int image_width = FreeImage_GetWidth(img);
	int image_height = FreeImage_GetHeight(img);

	// Convert the image to 24bit (BGR)
	FIBITMAP *img_temp=FreeImage_ConvertTo24Bits(img);
	FreeImage_Unload(img);
	img=img_temp;

	// Create a texture object
	unsigned int itex;
	glGenTextures(1, &itex);
	glBindTexture(GL_TEXTURE_2D, itex);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Set texture wrapping modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set the texture image as the image loaded from the file
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		image_width, image_height, 0,
		GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(img));

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Delete the image
	FreeImage_Unload(img);

	// Return the texture object name (index)
	return itex;
}

unsigned int LoadTextureCubeMapFromFile(
	const char *file_names[6])
// Create a cube map texture object and load its images from 6 image files
// file_names: (in) Image file name strings
// Return value: Name (index) of the texture object
{
	// Create a texture object
	unsigned int itex;
	glGenTextures(1, &itex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, itex);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Set texture wrapping modes
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Loop through all 6 faces of a cube
	for (int face=0; face<6; face++)
	{
		// Get the image file format
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // Image Format
		// Check the file signature and deduce its format
		fif = FreeImage_GetFileType(file_names[face], 0);
		// If unknown, try to guess the file format from the file extension
		if(fif == FIF_UNKNOWN) 
			fif = FreeImage_GetFIFFromFilename(file_names[face]);
		if(fif == FIF_UNKNOWN)
		{
			glDeleteTextures(1, &itex);
			return 0;
		}

		// Load the image from file
		FIBITMAP* img = FreeImage_Load(fif, file_names[face]);
		if (img==NULL)
		{
			glDeleteTextures(1, &itex);
			return 0;
		}

		// Get the image information
		int image_width = FreeImage_GetWidth(img);
		int image_height = FreeImage_GetHeight(img);

		// Convert the image to 24bit (BGR)
		FIBITMAP *img_temp=FreeImage_ConvertTo24Bits(img);
		FreeImage_Unload(img);
		img=img_temp;

		// Flip the image horizontally and vertically
		FreeImage_FlipHorizontal(img);
		FreeImage_FlipVertical(img);

		// Set the texture image of current face as the image loaded from the file
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 
			0, GL_RGB, 
			image_width, image_height, 0,
			GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(img));

		// Delete the image
		FreeImage_Unload(img);
	}

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Return the texture object name (index)
	return itex;
}

unsigned int LoadBumpMap2DFromHeightMapFile(const char *file_name)
// Load a height map image from an image file and create the corresponding bump map image
// Then create a 2D texture object and set the bump map image as its texture image
// file_name: (in) Height map image file name string
// Return value: Name (index) of the texture object
{
	// Get the height map image file format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // Image Format
	// Check the file signature and deduce its format
	fif = FreeImage_GetFileType(file_name, 0);
	// If unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(file_name);
	if(fif == FIF_UNKNOWN)
		return 0;

	// Load the height map image from file
	FIBITMAP* img = FreeImage_Load(fif, file_name);
	if (img==NULL) return 0;

	// Get the height map image information
	int image_width = FreeImage_GetWidth(img);
	int image_height = FreeImage_GetHeight(img);

	// Convert the height map image to 8bit (grey scale)
	FIBITMAP *img_temp=FreeImage_ConvertTo8Bits(img);
	FreeImage_Unload(img);
	img=img_temp;

	// Get pointer to the pixels of the height map image
	BYTE *height_image=FreeImage_GetBits(img);

	// Create a bump map image
	float *bump_image=new float [image_width*image_height*3];

	// Set pixel values of the bump map image using
	//   finite differences of pixel values of the height map image
	int i, j, k;
	float tx, ty;
	k=0;
	for (j=0; j<image_height; ++j)
	{
		for (i=0; i<image_width; ++i, ++k)
		{
			if (i==0)
				tx=height_image[k]-height_image[k+1];
			else if (i==image_width-1)
				tx=height_image[k-1]-height_image[k];
			else
				tx=0.5f*(height_image[k-1]-height_image[k+1]);

			if (j==0)
				ty=height_image[k]-height_image[k+image_width];
			else if (j==image_height-1)
				ty=height_image[k-image_width]-height_image[k];
			else
				ty=0.5f*(height_image[k-image_width]
					-height_image[k+image_width]);

			bump_image[k*3]=tx/255.0f;
			bump_image[k*3+1]=ty/255.0f;
			bump_image[k*3+2]=1.0f;
		}
	}

	// Create a texture object
	unsigned int itex;
	glGenTextures(1, &itex);
	glBindTexture(GL_TEXTURE_2D, itex);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	// Set texture wrapping modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set the texture image as the bump map image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 
		image_width, image_height, 0,
		GL_RGB, GL_FLOAT, bump_image);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Delete the height map image
	FreeImage_Unload(img);

	// Delete the bump map image
	delete [] bump_image;

	// Return the texture object name (index)
	return itex;
}
