#pragma once

#include "utils/debug.h"
#include<glm/glm.hpp>
#include<vector>

// Structure to standardize the vertices used in the meshes
// Packaging of tex and color coords into vertex
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texUV;
};



class VBO
{
public:
    // Reference ID of the Vertex Buffer Object
    GLuint ID;
    // Constructor that generates a Vertex Buffer Object and links it to vertices
    VBO(std::vector<Vertex>& vertices);
    VBO(std::vector<glm::mat4>& mat4s);

    // Binds the VBO
    void Bind();
    // Unbinds the VBO
    void Unbind();
    // Deletes the VBO
    void Delete();
};
