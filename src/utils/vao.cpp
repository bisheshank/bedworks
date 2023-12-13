#include"VAO.h"

// Constructor that generates a VAO ID
VAO::VAO()
{
    glGenVertexArrays(1, &ID);
    Debug::glErrorCheck();
}

// Links a VBO Attribute such as a position or color to the VAO
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
    VBO.Bind();
    Debug::glErrorCheck();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    Debug::glErrorCheck();
    glEnableVertexAttribArray(layout);
    Debug::glErrorCheck();
    VBO.Unbind();
    Debug::glErrorCheck();
}

// Binds the VAO
void VAO::Bind()
{
    glBindVertexArray(ID);
    Debug::glErrorCheck();
}

// Unbinds the VAO
void VAO::Unbind()
{
    glBindVertexArray(0);
    Debug::glErrorCheck();
}

// Deletes the VAO
void VAO::Delete()
{
    glDeleteVertexArrays(1, &ID);
    Debug::glErrorCheck();
}
