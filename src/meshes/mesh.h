#pragma once

#include"utils/debug.h"
#include<string>
#include <vector>
#include<glm/gtc/type_ptr.hpp>
#include"Texture.h"
#include"utils/VAO.h"
#include"utils/EBO.h"

class Mesh
{
public:
    std::vector <Vertex> vertices;
    std::vector <GLuint> indices;
    std::vector <Texture> textures;
    // Store VAO in public so it can be used in the Draw function
    VAO VAO;

    // Holders for VBOs so we can delete them
    GLuint instance_VBO = 0;
    GLuint vertices_VBO = 0;
    GLuint indices_EBO = 0;

    // Holds number of instances (if 1 the mesh will be rendered normally)
    unsigned int instances;

    // Initializes the mesh
    Mesh
        (
            std::vector <Vertex>& vertices,
            std::vector <GLuint>& indices,
            std::vector <Texture>& textures,
            unsigned int instances = 1,
            std::vector <glm::mat4> instanceMatrix = {}
            );

    // Draws the mesh
    void Draw
        (
            Shader& shader,
            glm::mat4 matrix = glm::mat4(1.0f),
            glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)
            );

    // Updates the instance matrices so you can have a new number of instances
    void updateInstances(unsigned int new_instances, std::vector<glm::mat4> new_instance_matrix);

    // Deletes all associated OpenGL memory with this object
    void cleanup();
};
