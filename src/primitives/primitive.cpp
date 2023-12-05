#include "Primitive.h"

Primitive::Primitive(const glm::mat4 &pctm, const ScenePrimitive &pprimitive) {
    // Initialize model matrix (from param)
    m_model = pctm;
    m_inverse_model = glm::inverse(m_model);

    // Initialize the normal transforming matrix
    m_inverse_normal = glm::inverse(glm::transpose(glm::mat3(m_model)));

    // Store relevant data for coloring/lighting in relevant fields
    m_ambient = pprimitive.material.cAmbient;
    m_diffuse = pprimitive.material.cDiffuse;
    m_specular = pprimitive.material.cSpecular;
    m_shininess = pprimitive.material.shininess;
}

// Default constructor
Primitive::Primitive() {
    // Give everything junk default values
    m_model = glm::mat4(1.0f);
    m_inverse_model = glm::mat4(1.0f);
    m_inverse_normal = glm::mat3(1.0f);

    // Junk lighting data
    m_ambient = glm::vec4(1.0f);
    m_diffuse = glm::vec4(1.0f);
    m_specular = glm::vec4(1.0f);
    m_shininess = 1.0f;
}

// Easily converts from cylindrical to cartesian coordinates
glm::vec3 Primitive::cylinder_to_cartesian(float theta, float r, float h) {
    // Using:
    // x = sin(theta) * r
    // y = h
    // z = cos(theta) * r
    return glm::vec3(glm::sin(theta) * r, h, glm::cos(theta) * r);
}

// Function to easily convert spherical coordinates to cartesian
glm::vec3 Primitive::spherical_to_cartesian(float theta, float phi, float r) {
    // Using:
    // x = sin(theta) * sin(phi) * r
    // y = cos(phi) * r
    // z = cos(theta) * sin(phi) * r
    return glm::vec3(glm::sin(theta) * glm::sin(phi) * r, glm::cos(phi) * r, glm::cos(theta) * glm::sin(phi) * r);
}

// Inserts the elements of a vec3 individually into a vector. Used when generating meshes
void Primitive::insert_vec3(std::vector<GLfloat> &data, glm::vec3 vec) {
    data.push_back(vec.x);
    data.push_back(vec.y);
    data.push_back(vec.z);
}

// Draws the shape in OpenGL (yay, we're using OpenGL now!)
// NOTE: Assumes the shader is already bound (this is an expensive operation)
// Also assume that camera and projection matrices have already been passed in
// Assumes relevant VAO has already been bound
void Primitive::draw(GLuint shader) {
    // Do not attempt to draw if VAO or VBO does not exist (the mesh hasn't been generated)
    assert(get_vao() != 0);
    assert(get_vbo() != 0);

    // Pass in necessary uniforms
    // Note that the view and projection matrices (from Camera) should already be passed in
    GLuint location;

    // Pass in model matrix
    location = glGetUniformLocation(shader, "model_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &(m_model[0][0]));
    Debug::glErrorCheck();

    // Pass in upper 3x3 transpose inverse for normal computation
    location = glGetUniformLocation(shader, "inverse_model_normal_matrix");
    Debug::glErrorCheck();
    glUniformMatrix3fv(location, 1, GL_FALSE, &(m_inverse_normal[0][0]));
    Debug::glErrorCheck();

    // Pass in relevant uniforms for primitive
    location = glGetUniformLocation(shader, "ambient");
    Debug::glErrorCheck();
    glUniform4f(location, m_ambient[0], m_ambient[1], m_ambient[2], m_ambient[3]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(shader, "diffuse");
    Debug::glErrorCheck();
    glUniform4f(location, m_diffuse[0], m_diffuse[1], m_diffuse[2], m_diffuse[3]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(shader, "specular");
    Debug::glErrorCheck();
    glUniform4f(location, m_specular[0], m_specular[1], m_specular[2], m_specular[3]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(shader, "shininess");
    Debug::glErrorCheck();
    glUniform1f(location, m_shininess);
    Debug::glErrorCheck();

    // Then draw it
    glDrawArrays(GL_TRIANGLES, 0, get_triangles() * 3);
    Debug::glErrorCheck();
}
