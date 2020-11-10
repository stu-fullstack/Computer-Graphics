#ifndef _IMAGE_LIB_
#define _IMAGE_LIB_

unsigned int LoadTexture2DFromFile(const char *file_name);
// Create a 2D texture object and load its image from an image file
// file_name: (in) Image file name string
// Return value: Name (index) of the texture object

unsigned int LoadTextureCubeMapFromFile(
	const char *file_names[6]);
// Create a cube map texture object and load its images from 6 image files
// file_names: (in) Image file name strings
// Return value: Name (index) of the texture object

unsigned int LoadBumpMap2DFromHeightMapFile(const char *file_name);
// Load a height map image from an image file and create the corresponding bump map image
// Then create a 2D texture object and set the bump map image as its texture image
// file_name: (in) Height map image file name string
// Return value: Name (index) of the texture object

#endif
