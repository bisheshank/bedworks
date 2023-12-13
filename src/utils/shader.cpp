#include"shader.h"

// Default constructor
Shader::Shader() {
    ID = 0;
}

// Constructor that also loads shader data
Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    ID = ShaderLoader::createShaderProgram(vertexFile, fragmentFile);
}

// Load data for shader
void Shader::loadData(const char* vertexFile, const char* fragmentFile)
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
