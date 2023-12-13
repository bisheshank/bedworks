#include"VBO.h"

// Constructor that generates a Vertex Buffer Object and links it to vertices
VBO::VBO(std::vector<Vertex>& vertices)
{
    glGenBuffers(1, &ID);
    Debug::glErrorCheck();
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    Debug::glErrorCheck();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();
}
VBO::VBO(std::vector<glm::mat4>& mat4s)
{
    glGenBuffers(1, &ID);
    Debug::glErrorCheck();
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    Debug::glErrorCheck();
    glBufferData(GL_ARRAY_BUFFER, mat4s.size() * sizeof(glm::mat4), mat4s.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();
}

// Binds the VBO
void VBO::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    Debug::glErrorCheck();
}

// Unbinds the VBO
void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
}

// Deletes the VBO
void VBO::Delete()
{
    glDeleteBuffers(1, &ID);
    Debug::glErrorCheck();
}
