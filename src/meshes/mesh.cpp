#include "Mesh.h"

Mesh::Mesh
    (
        std::vector <Vertex>& vertices,
        std::vector <GLuint>& indices,
        std::vector <Texture>& textures,
        unsigned int instances,
        std::vector <glm::mat4> instanceMatrix
        )
{
    Mesh::vertices = vertices;
    Mesh::indices = indices;
    Mesh::textures = textures;
    Mesh::instances = instances;

    VAO.Bind();
    // Generates Vertex Buffer Object and links it to vertices
    VBO instanceVBO(instanceMatrix);
    instance_VBO = instanceVBO.ID;
    VBO VBO(vertices);
    vertices_VBO = VBO.ID;
    // Generates Element Buffer Object and links it to indices
    EBO EBO(indices);
    indices_EBO = EBO.ID;
    // Links VBO attributes such as coordinates and colors to VAO
    VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
    if (instances != 1)
    {
        instanceVBO.Bind();
        // Can't link to a mat4 so you need to link four vec4s
        VAO.LinkAttrib(instanceVBO, 4, 4, GL_FLOAT, sizeof(glm::mat4), (void*)0);
        VAO.LinkAttrib(instanceVBO, 5, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
        VAO.LinkAttrib(instanceVBO, 6, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        VAO.LinkAttrib(instanceVBO, 7, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        // Makes it so the transform is only switched when drawing the next instance
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
    }
    // Unbind all to prevent accidentally modifying them
    VAO.Unbind();
    VBO.Unbind();
    instanceVBO.Unbind();
    EBO.Unbind();
}

// NOTE: Draw function should be called only after relevant camera uniforms (and lights) have been passed in
void Mesh::Draw
    (
        Shader& shader,
        glm::mat4 matrix,
        glm::vec3 translation,
        glm::quat rotation,
        glm::vec3 scale
        )
{
    // Bind shader to be able to access uniforms
    shader.Activate();
    VAO.Bind();

    // Keep track of how many of each type of textures we have
    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        std::string num;
        std::string type = textures[i].type;
        if (type == "diffuse")
        {
            num = std::to_string(numDiffuse++);
        }
        else if (type == "specular")
        {
            num = std::to_string(numSpecular++);
        }
        // This function sends the texture to the shader using the format:
        // diffuse0, diffuse1, (OR) specular0, specular1, etc.
        textures[i].texUnit(shader, (type + num).c_str(), i);
        textures[i].Bind();
    }

    // Check if instance drawing should be performed
    if (instances == 1)
    {
        // Initialize matrices
        glm::mat4 trans = glm::mat4(1.0f);
        glm::mat4 rot = glm::mat4(1.0f);
        glm::mat4 sca = glm::mat4(1.0f);

        // Transform the matrices to their correct form
        trans = glm::translate(trans, translation);
        rot = glm::mat4_cast(rotation);
        sca = glm::scale(sca, scale);

        // Push the matrices to the vertex shader
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(rot));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(sca));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));

        // Draw the actual mesh
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instances);
    }
}

// Updates the instance matrices so you can have a new number of instances
void Mesh::updateInstances(unsigned int new_instances, std::vector<glm::mat4> new_instance_matrix) {
    instances = new_instances;

    // Overwrite old VBO data with new instance matrix data
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
    Debug::glErrorCheck();
    glBufferData(GL_ARRAY_BUFFER, new_instance_matrix.size() * sizeof(glm::mat4), new_instance_matrix.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();
}

// Deletes all associated OpenGL memory with this object
void Mesh::cleanup() {
    VAO.Delete();
    glDeleteBuffers(1, &instance_VBO);
    Debug::glErrorCheck();
    glDeleteBuffers(1, &vertices_VBO);
    Debug::glErrorCheck();
    glDeleteBuffers(1, &indices_EBO);
    Debug::glErrorCheck();
}
