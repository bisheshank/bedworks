#include"shader.h"

// Constructor that build the Shader Program from 2 different shaders
Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
    // Uses the shaderloader code given to us to make the shader
    ID = ShaderLoader::createShaderProgram(vertexFile, fragmentFile);
    Debug::glErrorCheck();

}

// Activates the Shader Program
void Shader::Activate()
{
    glUseProgram(ID);
    Debug::glErrorCheck();
}

// Deactivates Shader Program
void Shader::Deactivate()
{
    glUseProgram(0);
    Debug::glErrorCheck();
}

// Deletes the Shader Program
void Shader::Delete()
{
    glDeleteProgram(ID);
    Debug::glErrorCheck();
}
