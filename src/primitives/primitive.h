#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "GL/glew.h"
#include "utils/scenedata.h"
#include "glm/glm.hpp"
#include "utils/debug.h"
#include "numbers"

class Primitive
{
public:
    // Constructor
    Primitive(const glm::mat4 &pctm, const ScenePrimitive &pprimitive);
    Primitive();

    // Function to draw this primitive in OpenGL
    void draw(GLuint shader);

    // Generates mesh for this primitive and binds VAOs and VBOs to it
    virtual void generate_mesh(const int shape_param_1, const int shape_param_2) = 0;

    // Deletes all VAOs and VBOs associated with this primitive
    virtual void delete_buffers() = 0;

    // (Fake?) destructor
    virtual ~Primitive() = default;


protected:

    // Function to retrieve VAO for this primitive class
    virtual GLuint get_vao() = 0;

    // Function to retrieve VBO for this primitive class
    virtual GLuint get_vbo() = 0;

    // Function to retrieve number of triangles for this class
    virtual int get_triangles() = 0;

    // Inserts a vec3 into a vector of floats (useful for generating meshes)
    void insert_vec3(std::vector<GLfloat> &data, glm::vec3 vec);

    // Helper functions for coordinate conversions to be used by children
    glm::vec3 cylinder_to_cartesian(float theta, float r, float h);
    glm::vec3 spherical_to_cartesian(float theta, float phi, float r);

    // Fields we want all inherited subclasses to have access to

    // Model matrix (CTM)
    glm::mat4 m_model;

    // Inverse of model matrix
    glm::mat4 m_inverse_model;

    // Matrix for transforming normal vectors
    glm::mat3 m_inverse_normal;

    // Fields to determine color for ambient, diffuse, specular
    glm::vec4 m_ambient;
    glm::vec4 m_diffuse;
    glm::vec4 m_specular;

    // Shininess (also for specular)
    float m_shininess;
};

#endif // PRIMITIVE_H
