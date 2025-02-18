#pragma once

#include "utils/debug.h"
#include "libraries/include/stb/stb_image.h"
#include "utils/shader.h"

class Texture
{
public:
    GLuint ID;
    const char* type;
    GLuint unit;

    Texture(const char* image, const char* texType, GLuint slot);

    // Assigns a texture unit to a texture stored in inputted uniform
    void texUnit(Shader shader, const char* uniform, GLuint unit);
    // Binds a texture
    void Bind();
    // Unbinds a texture
    void Unbind();
    // Deletes a texture
    void Delete();
};
