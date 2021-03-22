#include <GL/glew.h>
#include <cstdio>
#include <iostream>

GLuint loadBmp(const char *image_path)
{
	FILE *file = fopen(image_path, "rb");
	// open the file
	if (!file)
	{
		std::cerr << image_path << " could not be opened." << std::endl;
		return 0;
	}

	// read the header, first 54 bytes
	const unsigned hdr_size = 54;
	unsigned char header[hdr_size];
	// if less than 54 bytes are read, problem
	if (fread(header, 1, hdr_size, file) != hdr_size)
	{ 
		std::cerr << image_path <<  " is not a well-formed BMP file." << std::endl;
		fclose(file);
		return 0;
	}
	// check magic number ("BM")
	if (header[0] !='B' || header[1] != 'M')
	{
		std::cerr << image_path << " is not a well-formed BMP file." << std::endl;
		fclose(file);
		return 0;
	}

	// check for 24bpp
	if (*(int*)&header[0x1E] != 0)
	{
		std::cerr << image_path << " is not a well-formed BMP file." << std::endl;
		fclose(file);
		return 0;
	}
	if (*(int*)&header[0x1C] != 24)
	{
		std::cerr << image_path << " is not a well-formed BMP file." << std::endl;
		fclose(file);
		return 0;
	}

	// extract header fields
	unsigned dataPos = *(int*)&header[0x0A];
	if (dataPos == 0)
		dataPos = hdr_size;

	const unsigned width  = *(int*)&header[0x12];
	const unsigned height = *(int*)&header[0x16];

	unsigned image_size = *(int*)&header[0x22];
	// double check image size
	if (image_size == 0)
		image_size = width * height * 3;

	// actual RGB data
	unsigned char *data = new unsigned char[image_size];

	// read data from the file into the buffer
	fread(data, 1, image_size, file);

	// everything is in memory now, the file can be closed
	fclose(file);

	// create one OpenGL texture
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	// Bind the newly created texture: all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// return the ID of the texture we just created
	return texture_id;
}

