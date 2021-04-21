// needs to be before GL
#include <GL/glew.h>
#include "controls.hpp"
#include "loadBmp.h"
#include "loadShaders.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

void printIndent(unsigned level)
{
	for (unsigned i = 0; i < 3*level; ++i)
		std::cout << ' ';
}

void dumpNode(const aiNode *node, unsigned indent)
{
	printIndent(indent);
	std::cout << "Name: " << node->mName.C_Str() << std::endl;

	printIndent(indent);
	std::cout << "NumChildren: " << node->mNumChildren << std::endl;

	printIndent(indent);
	std::cout << "NumMeshes: " << node->mNumMeshes << std::endl;

	for (unsigned i = 0; i < node->mNumChildren; ++i)
		dumpNode(node->mChildren[i], indent+1);
}

void dumpScene(const aiScene *scene)
{
	dumpNode(scene->mRootNode, 0);
	std::cout << std::endl;

	std::cout << "Materials: " << scene->mNumMaterials << std::endl;
	std::cout << "NumAnimations: " << scene->mNumAnimations << std::endl;
	for (unsigned i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation *anim = scene->mAnimations[i];
		// nName is often empty
		std::cout << "   Duration/Rate: "
		    << anim->mDuration
		    << '/' << anim->mTicksPerSecond
		    << std::endl;
		std::cout << "   Node Channels: " << anim->mNumChannels << std::endl;
		std::cout << "   Mesh Channels: " << anim->mNumMeshChannels << std::endl;
		std::cout << "   Mesh Morph Channels: " << anim->mNumMorphMeshChannels << std::endl;
	}

	std::cout << "Meshes: " << scene->mNumMeshes << std::endl;
	for (unsigned i = 0; i < scene->mNumMeshes; ++i)
	{
		std::cout << i << std::endl;
		const aiMesh *paiMesh = scene->mMeshes[i];
		std::cout << "   MatInd: " << paiMesh->mMaterialIndex << std::endl;
		std::cout << "   NumBones: " << paiMesh->mNumBones << std::endl;
		std::cout << "   NumAnimMeshes: " << paiMesh->mNumAnimMeshes << std::endl;

		std::cout << "   NumVert: " << paiMesh->mNumVertices << std::endl;
		for (unsigned j = 0; j < paiMesh->mNumVertices; ++j)
		{
			std::cout << "      " << j << std::endl;
			const aiVector3D *pPos = &paiMesh->mVertices[j];
			std::cout << "      Pos "
			    << pPos->x << ' '
			    << pPos->y << ' '
			    << pPos->z << std::endl;

			if (paiMesh->HasTextureCoords(0)) {
				const aiVector3D *pTexCoord = &paiMesh->mTextureCoords[0][j];
				std::cout << "      Tex "
				    << pTexCoord->x << ' '
				    << pTexCoord->y << ' '
				    << pTexCoord->z << std::endl;
			}
			const aiVector3D *pNormal = &paiMesh->mNormals[j];
			std::cout << "      Norm "
			    << pNormal->x << ' '
			    << pNormal->y << ' '
			    << pNormal->z << std::endl;
		}
	}
}

//----------------------------------------------------------------------------
/// One mesh in the scene
class Mesh
{
public:
	Mesh() = default;

	Mesh(GLuint vertex_buffer,
	     GLuint uv_buffer,
	     GLuint normal_buffer,
	     GLuint face_buffer,
	     unsigned num_faces)
	: vertex_buffer_(vertex_buffer)
	, uv_buffer_(uv_buffer)
	, normal_buffer_(normal_buffer)
	, face_buffer_(face_buffer)
	, num_faces_(num_faces)
	{
	}

	~Mesh()
	{
		glDeleteBuffers(1, &vertex_buffer_);
		glDeleteBuffers(1, &uv_buffer_);
		glDeleteBuffers(1, &normal_buffer_);
		glDeleteBuffers(1, &face_buffer_);
	}

	void render()
	{
		// attribute 0 - vertex data
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// draw the triangles!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer_);
		glDrawElements(GL_TRIANGLES, num_faces_, GL_UNSIGNED_INT, (void*)0);

		// done
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

protected: // data
	GLuint vertex_buffer_ = 0; ///< vertex buffer id
	GLuint uv_buffer_ = 0;     ///< uv buffer id
	GLuint normal_buffer_ = 0; ///<  normal buffer id
	GLuint face_buffer_ = 0;   ///< faces hold indexes of vertexes
	unsigned num_faces_ = 0;
};

bool hasMeshes(const aiNode *node)
{
	if (node->mNumMeshes > 0)
		return true;

	for (unsigned i = 0; i < node->mNumChildren; ++i)
	{
		if (hasMeshes(node->mChildren[i]))
			return true;
	}

	return false;
}

class Node
{
public:
	Node() = default;

	void resolveMeshes(const aiNode *node, const std::vector<Mesh*> &meshes)
	{
		name_ = std::string(node->mName.C_Str());
		std::cout << "Creating node " << name_ << std::endl;

		for (unsigned i = 0; i < node->mNumChildren; ++i)
		{
			aiNode *child = node->mChildren[i];
			if (hasMeshes(child))
			{
				Node *child_node = new Node;
				child_node->resolveMeshes(child, meshes);
				children_.push_back(child_node);
			}
		}

		// pull mesh pointers from global mesh array via index
		for (unsigned i = 0; i < node->mNumMeshes; ++i)
		{
			const unsigned mesh_idx = node->mMeshes[i];
			meshes_.push_back(meshes[mesh_idx]);
		}
	}

	void render()
	{
		// TODO apply matrix transformation
		for (const auto &m : meshes_)
			m->render();

		for (const auto &n : children_)
			n->render();
	}

protected:
	std::string name_; ///< name (used in animation)
	glm::mat4 transform_; ///< relative to parent
	std::vector<Node*> children_; ///< tree of children
	std::vector<Mesh*> meshes_; ///< meshes on this node
};

class Scene
{
public:
	Scene() = default;

	bool load(const char *obj_path)
	{
		Assimp::Importer importer;
		// TODO: other flags? configuration?
		const unsigned flags = aiProcess_CalcTangentSpace
		    | aiProcess_Triangulate
		    | aiProcess_JoinIdenticalVertices
		    | aiProcess_SortByPType;
		const aiScene *scene = importer.ReadFile(obj_path, flags);
		if (!scene)
		{
			std::cerr << "Failed to import " << obj_path << std::endl;
			return false;
		}

		// TODO: materials
		// TODO: animations

		// meshes
		for (unsigned i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh *paiMesh = scene->mMeshes[i];
			if (paiMesh->mNumAnimMeshes != 0)
			{
				std::cerr << "Mesh " << i << " with animations!" << std::endl;
			}
			// TODO: handle other texture maps...
			const bool has_texture_coords = paiMesh->HasTextureCoords(0);
			const bool has_normals = paiMesh->HasNormals();

			const glm::vec3 zero(0.f, 0.f, 0.f);
			std::vector<glm::vec3> vertex_buffer_data;
			std::vector<glm::vec2> uv_buffer_data;
			std::vector<glm::vec3> normal_buffer_data;

			for (unsigned j = 0; j < paiMesh->mNumVertices; ++j)
			{
				const aiVector3D *pPos = &paiMesh->mVertices[j];
				vertex_buffer_data.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));

				if (has_texture_coords)
				{
					const aiVector3D *pTexCoord = &paiMesh->mTextureCoords[0][j];
					uv_buffer_data.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
				}
				else
				{
					uv_buffer_data.push_back(glm::vec2(zero.x, zero.y));
				}

				if (has_normals)
				{
					const aiVector3D *pNormal = &paiMesh->mNormals[j];
					normal_buffer_data.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
				}
				else
				{
					normal_buffer_data.push_back(glm::vec3(zero.x, zero.y, zero.z));
				}
			}

			std::vector<unsigned> face_buffer_data;
			for (unsigned i = 0; i < paiMesh->mNumFaces; ++i)
			{
				const aiFace &face = paiMesh->mFaces[i];
				for (unsigned j = 0; j < face.mNumIndices; ++j)
				{
					face_buffer_data.push_back(face.mIndices[j]);
				}
			}

			GLuint vertex_buffer;
			glGenBuffers(1, &vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(glm::vec3), &vertex_buffer_data[0], GL_STATIC_DRAW);

			GLuint uv_buffer;
			glGenBuffers(1, &uv_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glBufferData(GL_ARRAY_BUFFER, uv_buffer_data.size() * sizeof(glm::vec2), &uv_buffer_data[0], GL_STATIC_DRAW);

			GLuint normal_buffer;
			glGenBuffers(1, &normal_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
			glBufferData(GL_ARRAY_BUFFER, normal_buffer_data.size() * sizeof(glm::vec3), &normal_buffer_data[0], GL_STATIC_DRAW);

			GLuint face_buffer;
			glGenBuffers(1, &face_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_buffer_data.size() * sizeof(face_buffer_data[0]), &face_buffer_data[0], GL_STATIC_DRAW);

			meshes_.push_back(new Mesh(vertex_buffer, uv_buffer, normal_buffer, face_buffer, face_buffer_data.size()));
		}

		root_ = new Node;
		root_->resolveMeshes(scene->mRootNode, meshes_);

		return true;
	}

	void render()
	{
		// TODO pass in MVP
		root_->render();
	}

private:
	Node *root_ = nullptr;
	std::vector<Mesh*> meshes_;
};

int main(int argc, char **argv)
{
	if (!glfwInit())
	{
		std::cerr << "Failed GLFW init" << std::endl;
		return 1;
	}

	const char *obj_path = "../cube.obj";
	if (argc > 1)
	{
		obj_path = argv[1];
	}

	/*
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(obj_path,
	    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!scene)
	{
		std::cerr << "Failed to import " << obj_path << std::endl;
		return 2;
	}
	dumpScene(scene);
	return 0;
	*/

	// create main window
	GLFWwindow *window = glfwCreateWindow( 1024, 768, "YingYang", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 2;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW init failed" << std::endl;
		glfwTerminate();
		return 3;
	}

	Scene main_scene;
	if (!main_scene.load(obj_path))
	{
		return 4;
	}

	Controls *controls = new Controls(window);

	// make sure we get all key presses
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// hide mouse cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// background color (blue .4)
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// enable depth test
	glEnable(GL_DEPTH_TEST);
	// accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// read and compile shaders
	GLuint program_id = loadShaders("../standardShading.vert.glsl", "../standardShading.frag.glsl");

	// get a handle for our "MVP" uniform
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
	GLuint view_matrix_id = glGetUniformLocation(program_id, "V");
	GLuint model_matrix_id = glGetUniformLocation(program_id, "M");

	// load the texture using any two methods
	GLuint texture = loadBmp("../uvtemplate.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");

	// get a handle for our texture sampler uniform
	GLuint TextureID  = glGetUniformLocation(program_id, "myTextureSampler");

	glUseProgram(program_id);
	GLuint light_id = glGetUniformLocation(program_id, "LightPosition_worldspace");

	do
	{
		// erase screen before drawing
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program_id);

		// Compute the MVP matrix from keyboard and mouse input
		controls->computeMatricesFromInputs();
		glm::mat4 projection_matrix = controls->projectionMatrix();
		glm::mat4 view_matrix = controls->viewMatrix();
		glm::mat4 model_matrix = glm::mat4(1.0);
		glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

		// set model view projection matrix
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &model_matrix[0][0]);
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, &view_matrix[0][0]);

		glm::vec3 light_pos = glm::vec3(4,4,4);
		glUniform3f(light_id, light_pos.x, light_pos.y, light_pos.z);

		main_scene.render();

		// done drawing! swap buffer to front
		glfwSwapBuffers(window);
		glfwPollEvents(); // get events

		// while not escape key, or close window button
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	         glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteTextures(1, &texture);
	glDeleteProgram(program_id);

	delete controls;
	glfwTerminate();
	return 0;
}

