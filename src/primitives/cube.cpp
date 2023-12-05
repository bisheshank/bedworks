#include "cube.h"

Cube::Cube(const glm::mat4 &pctm, const ScenePrimitive &pprimitive) : Primitive(pctm, pprimitive)
{
    // Nothing else to be done in Cube constructor
}

// Constructor (does nothing different from base class)
Cube::Cube() : Primitive()
{
    // Do nothing (nothing else needed to construct)
}

// Destructor
Cube::~Cube() {
    // Then if there are no more active Cubes, delete the VAOs and VBOs
}

// Function to delete buffers
void Cube::delete_buffers() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        Debug::glErrorCheck();
        m_vao = 0;
    }

    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        Debug::glErrorCheck();
        m_vbo = 0;
    }
}

// Helper function to make a tile (sector) of a Cube. Used in making mesh
void Cube::make_tile(std::vector<GLfloat> &data, glm::vec3 top_left, glm::vec3 top_right, glm::vec3 bottom_left, glm::vec3 bottom_right) {
    // Vertices are inserted with coordinates first, followed by their normals

    // Normal can be computed by normalizing CB - CA
    // (1st - 3rd) X (2nd - 3rd)
    // This normal is the same (I think) for both triangles
    // (bottomLeft - bottomRight) X (topLeft - bottomRight)
    glm::vec3 normal = glm::normalize(glm::cross((top_left - bottom_right), (bottom_left - bottom_right)));

    // Create the first triangle
    insert_vec3(data, top_left);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_left);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_right);
    insert_vec3(data, normal);

    num_triangles += 1;

    // Second triangle
    insert_vec3(data, top_right);
    insert_vec3(data, normal);
    insert_vec3(data, top_left);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_right);
    insert_vec3(data, normal);

    num_triangles += 1;

}

// Helper function to make a wedge (slice) of a Cube. Used in making mesh
void Cube::make_face(std::vector<GLfloat> &data, glm::vec3 top_left, glm::vec3 top_right, glm::vec3 bottom_left, glm::vec3 bottom_right, int shape_parameter_1) {
    // Start from Left --> Right
    glm::vec3 horizontal = bottom_right - bottom_left;
    horizontal = horizontal * (1.0f / shape_parameter_1);

    // Start from Top --> Down
    glm::vec3 vertical = bottom_left - top_left;
    vertical = vertical * (1.0f / shape_parameter_1);

    // A "face" is just a tile but further subtiled
    for (int row = 0; row < shape_parameter_1; row++) {
        for (int col = 0; col < shape_parameter_1; col++) {
            // Translate the vectors that define the face
            glm::vec3 translated_top_left = top_left + ((float) row * horizontal) + ((float) col * vertical);
            glm::vec3 translated_top_right = top_left + ((float) (row + 1) * horizontal) + ((float) col * vertical);
            glm::vec3 translated_bottom_left = top_left + ((float) row * horizontal) + ((float) (col + 1) * vertical);
            glm::vec3 translated_bottom_right = top_left + ((float) (row + 1) * horizontal) + ((float) (col + 1) * vertical);

            // Make this iteration of the tile
            make_tile(data, translated_top_left, translated_top_right, translated_bottom_left, translated_bottom_right);
        }
    }
}

// Generates mesh for this primitive and binds VAOs and VBOs to it
// Don't call this function repeatedly. It will generate a new mesh each time.
// Call ONCE to generate a new mesh for all Cubes (when primitive params change)
void Cube::generate_mesh(const int shape_param_1, const int shape_param_2) {
    // Clip the shape parameter appropriately
    int new_param_1 = std::max(shape_param_1, 1);

    // Don't generate mesh if params haven't changed
    if (shape_parameter_1 == new_param_1) {
        return;
    }

    // Update this mesh's parameters to match
    shape_parameter_1 = new_param_1;

    // General note: Vertices are stored as VERTEX COORD, NORMAL VEC in memory

    // Reset static variables
    num_triangles = 0;

    // For the VAOs/VBOs, free those buffers if they exist (we'll be recreating them)
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        Debug::glErrorCheck();
        m_vao = 0;
    }

    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        Debug::glErrorCheck();
        m_vbo = 0;
    }

    // Time to generate the new mesh!
    // First, generate a vector that contains all the vertex data and normals we want
    std::vector<GLfloat> data;

    // 6 Faces, defined with 8 vertices
    glm::vec3 pt1(0.5f, 0.5f, 0.5f);
    glm::vec3 pt2(0.5f, 0.5f, -0.5f);
    glm::vec3 pt3(0.5f, -0.5f, 0.5f);
    glm::vec3 pt4(0.5f, -0.5f, -0.5f);
    glm::vec3 pt5(-0.5f, 0.5f, 0.5f);
    glm::vec3 pt6(-0.5f, 0.5f, -0.5f);
    glm::vec3 pt7(-0.5f, -0.5f, 0.5f);
    glm::vec3 pt8(-0.5f, -0.5f, -0.5f);

    // Make the faces!
    make_face(data, pt6, pt2, pt5, pt1, shape_param_1);
    make_face(data, pt1, pt2, pt3, pt4, shape_param_1);
    make_face(data, pt7, pt3, pt8, pt4, shape_param_1);
    make_face(data, pt6, pt5, pt8, pt7, shape_param_1);
    make_face(data, pt5, pt1, pt7, pt3, shape_param_1);
    make_face(data, pt2, pt6, pt4, pt8, shape_param_1);

    // Create a new VBO for this data
    glGenBuffers(1, &m_vbo);
    Debug::glErrorCheck();

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    Debug::glErrorCheck();

    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();

    // Now create a new VAO for this data
    glGenVertexArrays(1, &m_vao);
    Debug::glErrorCheck();

    glBindVertexArray(m_vao);
    Debug::glErrorCheck();

    // Add position and normal components to this VAO (Set up the VAO)
    glEnableVertexAttribArray(0);
    Debug::glErrorCheck();
    glEnableVertexAttribArray(1);
    Debug::glErrorCheck();

    // Ok, actually add them
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    Debug::glErrorCheck();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    Debug::glErrorCheck();

    // Unbind the VBO and VBO from OpenGL state (they're saved in this object)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
    glBindVertexArray(0);
    Debug::glErrorCheck();
}

// Function to retrieve VAO for this primitive class
// Must be overriden to return the correct value (for this subclass only)
GLuint Cube::get_vao() {
    return m_vao;
}

// Function to retrieve VBO for this primitive class
// Must be overriden to return the correct value (for this subclass only)
GLuint Cube::get_vbo() {
    return m_vbo;
}

// Function to retrieve number of triangles for this class
// Must be overriden to return the correct value (for this subclass only)
int Cube::get_triangles() {
    return num_triangles;
}

// Initialize the static variables to something
GLuint Cube::m_vao = 0;
GLuint Cube::m_vbo = 0;
int Cube::num_triangles = 0;
int Cube::shape_parameter_1 = 0;
