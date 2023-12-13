#include"EBO.h"

// Constructor that generates a Elements Buffer Object and links it to indices
EBO::EBO(std::vector<GLuint>& indices)
{;
    glGenBuffers(1, &ID);
    Debug::glErrorCheck();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    Debug::glErrorCheck();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();
}

// Binds the EBO
void EBO::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    Debug::glErrorCheck();
}

// Unbinds the EBO
void EBO::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
}

// Deletes the EBO
void EBO::Delete()
{
    glDeleteBuffers(1, &ID);
    Debug::glErrorCheck();
}
