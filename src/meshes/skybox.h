#pragma once

#include "utils/debug.h"
#include "utils/shader.h"
#include "utils/vao.h"
#include "meshes/model.h"

class Skybox
{
public:
    // Basic no-arg constructor since realtime instance will have a member variable of type skybox
    Skybox();

    // Loads the texture to initialize skybox
    void load_texture(std::vector<std::string> faces);

    // Draws the skybox (sends any necessary uniforms from skybox to shader)
    void draw(Shader shader);

    // Cleanup any OpenGL memory
    void cleanup();

private:
    // VAOs, VBOs, and texture objects used to render this skybox
    GLuint cubemap_VAO;
    GLuint cubemap_VBO;
    GLuint cubemap_texture;
    GLuint cubemap_EBO;

    // Identifies if the skybox has been instantiated yet
    bool instantiated = false;
};

