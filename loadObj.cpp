#include <glm/glm.hpp>
#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

void extractAttributes(const std::string &v, unsigned *idx, unsigned *uv_idx, unsigned *normal_idx)
{
	const size_t i0 = v.find('/');
	std::istringstream v0(v.substr(0, i0));
	v0 >> *idx;

	const size_t i1 = v.find('/', i0+1);
	std::istringstream v1(v.substr(i0+1, i1 - i0 - 1));
	v1 >> *uv_idx;

	const size_t i2 = v.find('/', i1+1);
	std::istringstream v2(v.substr(i1+1, i2 - i1 - 1));
	v2 >> *normal_idx;
}

bool loadObj(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals,
    bool is_dds
)
{
	std::ifstream is(path);
	if (!is)
	{
		std::cerr << path << " could not be opened." << std::endl;
		return false;
	}

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::string line;
	for (std::getline(is, line); !is.eof(); std::getline(is, line))
	{
		// pull first word on line
		std::istringstream iss(line);
		std::string first_word;
		iss >> first_word;

		if (first_word == "v") // vertex
		{
			glm::vec3 vertex;
			iss >> vertex.x >> vertex.y >> vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (first_word == "vt") // vertex texture coordinate (uv)
		{
			glm::vec2 uv;
			iss >> uv.x >> uv.y;
			// Invert V coordinate with DDS texture, which are inverted
			if (is_dds)
				uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (first_word == "vn") // vertex normal
		{
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (first_word == "f") // face
		{
			// pull 3 vertices
			std::string v0, v1, v2;
			iss >> v0 >> v1 >> v2;

			// check for attributes
			size_t i0 = v0.find('/');
			if (i0 != std::string::npos)
			{
				unsigned vertex_index = 0, uv_index = 0, normal_index = 0;
				extractAttributes(v0, &vertex_index, &uv_index, &normal_index);
				vertexIndices.push_back(vertex_index);
				uvIndices    .push_back(uv_index);
				normalIndices.push_back(normal_index);

				extractAttributes(v1, &vertex_index, &uv_index, &normal_index);
				vertexIndices.push_back(vertex_index);
				uvIndices    .push_back(uv_index);
				normalIndices.push_back(normal_index);

				extractAttributes(v2, &vertex_index, &uv_index, &normal_index);
				vertexIndices.push_back(vertex_index);
				uvIndices    .push_back(uv_index);
				normalIndices.push_back(normal_index);
			}
			else // just the vertex index
			{
				std::istringstream isv0(v0);
				unsigned v0_int;
				isv0 >> v0_int;
				vertexIndices.push_back(v0_int);

				std::istringstream isv1(v1);
				unsigned v1_int;
				isv1 >> v1_int;
				vertexIndices.push_back(v1_int);

				std::istringstream isv2(v2);
				unsigned v2_int;
				isv1 >> v2_int;
				vertexIndices.push_back(v2_int);
			}
		}
	}

	// foreach vertex
	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
		// get indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	}

	return true;
}

