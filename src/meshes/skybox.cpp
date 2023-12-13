#include "skybox.h"

// Basic no-arg constructor since realtime instance will have a member variable of type skybox
Skybox::Skybox() {
    // Initialize the empty VBOs to 0 so we don't try to delete them
    cubemap_VBO = 0;
    cubemap_texture = 0;
    cubemap_EBO = 0;

    // Skybox hasn't been instantiated yet
    instantiated = false;
}

// Loads the texture to initialize skybox
// Note the order for the elements of the Skybox must be in:
// RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK
// According to this pattern: https://learnopengl.com/img/advanced/cubemaps_skybox.png
void Skybox::load_texture(std::vector<std::string> faces) {
    std::vector<GLfloat> skybox_vertices = {
        // positions of vertices
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    std::vector<GLuint> skybox_indices = {
        // Labels for vertices that make up each face
        // Right
        1, 2, 6,
        6, 5, 1,
        // Left
        0, 4, 7,
        7, 3, 0,
        // Top
        4, 5, 6,
        6, 7, 4,
        // Bottom
        0, 3, 2,
        2, 1, 0,
        // Back
        0, 1, 5,
        5, 4, 0,
        // Front
        3, 7, 6,
        6, 2, 3
    };

    // Create the necessary VAOs, VBOs, and EBOs for the skybox
    // Note that the VAO is a member of the Skybox class, so it has already been generated in the constructor
    // In that case, we generate the VBOs and EBOs
    glGenBuffers(1, &cubemap_VBO);
    Debug::glErrorCheck();
    glGenBuffers(1, &cubemap_EBO);
    Debug::glErrorCheck();

    // Fill these buffers with relevant data
    glBindBuffer(GL_ARRAY_BUFFER, cubemap_VBO);
    Debug::glErrorCheck();
    glBufferData(GL_ARRAY_BUFFER, skybox_vertices.size() * sizeof(GLfloat), skybox_vertices.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemap_EBO);
    Debug::glErrorCheck();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox_indices.size() * sizeof(GLuint), skybox_indices.data(), GL_STATIC_DRAW);

    // Now bind the VAO
    vao.Bind();
    Debug::glErrorCheck();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    Debug::glErrorCheck();
    glEnableVertexAttribArray(0);
    Debug::glErrorCheck();

    // Reset state by unbinding everything
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
    vao.Unbind();
    Debug::glErrorCheck();

    // Create the necessary texture infrastructure for cubemap
    glGenTextures(1, &cubemap_texture);
    Debug::glErrorCheck();
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    Debug::glErrorCheck();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Debug::glErrorCheck();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Debug::glErrorCheck();
    // These are very important to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();

    // Now we can finally start attaching textures to this cube map
    for (unsigned int i = 0; i < 6; i++) {
        int width;
        int height;
        int channels;
        // Use the stbi image library to extract the image
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

        // If we were able to successfully load, make the texture
        if (data) {
            stbi_set_flip_vertically_on_load(false);
            // Since we've already bound the cubemap, this will bind properly
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            // Done loading this texture
            stbi_image_free(data);
        }
        else {
            std::cerr << "Loading texture " << faces[i] << " failed\n";
            stbi_image_free(data);
        }
    }
}

// Draws the skybox (sends any necessary uniforms from skybox to shader)
void Skybox::draw(Shader shader) {
    // Change depth function for rendering skyboxes
    glDepthFunc(GL_LEQUAL);

    // Shader should have a "cubemap" field which contains the cubemap texture. Send the texture to it
    vao.Bind();
    glActiveTexture(GL_TEXTURE0);
    Debug::glErrorCheck();
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    Debug::glErrorCheck();

    // Y'know... draw the damn thing
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    Debug::glErrorCheck();
    vao.Unbind();

    // Switch depth function back
    glDepthFunc(GL_LESS);
    Debug::glErrorCheck();
}

// Cleanup any OpenGL memory
void Skybox::cleanup() {
    vao.Delete();

    // Only clean up the buffers/textures if they've actually been created
    if (cubemap_VBO != 0) {
        glDeleteBuffers(1, &cubemap_VBO);
        Debug::glErrorCheck();
    }

    if (cubemap_EBO != 0) {
        glDeleteBuffers(1, &cubemap_EBO);
        Debug::glErrorCheck();
    }

    if (cubemap_texture != 0) {
        glDeleteTextures(1, &cubemap_texture);
        Debug::glErrorCheck();
    }
}
