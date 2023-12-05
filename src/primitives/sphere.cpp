#include "sphere.h"

// Constructor (does nothing different from base class)
Sphere::Sphere(const glm::mat4 &pctm, const ScenePrimitive &pprimitive) : Primitive(pctm, pprimitive)
{
    // Do nothing (nothing else needed to construct)
}

// Constructor (does nothing different from base class)
Sphere::Sphere() : Primitive()
{
    // Do nothing (nothing else needed to construct)
}

// Destructor
Sphere::~Sphere() {
    // Then if there are no more active spheres, delete the VAOs and VBOs
}

// Function to delete buffers
void Sphere::delete_buffers() {
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

// Drawing function (does not render if params are small enough)
void Sphere::draw(GLuint shader) {
    // If minimum shape params are not met, do not draw
    if (shape_parameter_1 < 2 || shape_parameter_2 < 3) {
        return;
    }

    // Otherwise draw
    Primitive::draw(shader);
}

// Helper function to make a tile (sector) of a sphere. Used in making mesh
void Sphere::make_tile(std::vector<GLfloat> &data, glm::vec3 top_left, glm::vec3 top_right, glm::vec3 bottom_left, glm::vec3 bottom_right) {
    // Vertices are inserted with coordinates first, followed by their normals

    // Note that normals of a sphere are given by: <2x, 2y, 2z>, and then normalized (so we can drop the coeffs)

    // Create the first triangle
    insert_vec3(data, top_left);
    insert_vec3(data, glm::normalize(top_left));
    insert_vec3(data, bottom_left);
    insert_vec3(data, glm::normalize(bottom_left));
    insert_vec3(data, bottom_right);
    insert_vec3(data, glm::normalize(bottom_right));

    num_triangles += 1;

    // Second triangle
    insert_vec3(data, top_right);
    insert_vec3(data, glm::normalize(top_right));
    insert_vec3(data, top_left);
    insert_vec3(data, glm::normalize(top_left));
    insert_vec3(data, bottom_right);
    insert_vec3(data, glm::normalize(bottom_right));

    num_triangles += 1;
}

// Helper function to make a wedge (slice) of a sphere. Used in making mesh
void Sphere::make_wedge(std::vector<GLfloat> &data, float curr_theta, float next_theta, const int shape_param_1) {
    // Generate wedge from top to bottom, iterating along phi axis (spherical coordinates)
    float d_phi = std::numbers::pi / shape_param_1;
    // Iterates over tiles we want to create
    for (int sector = 0; sector < shape_param_1; sector++) {
        // Initialize angles
        float top_phi = d_phi * sector;
        float bot_phi = d_phi * (sector + 1);
        // Theta specifies how far horizontally we've traversed the sphere (again, spherical coordinates)
        float left_theta = curr_theta;
        float right_theta = next_theta;

        // Compute the points of the tile/sector we want to create
        glm::vec3 top_left = spherical_to_cartesian(left_theta, top_phi, 0.5f);
        glm::vec3 top_right= spherical_to_cartesian(right_theta, top_phi, 0.5f);
        glm::vec3 bot_left = spherical_to_cartesian(left_theta, bot_phi, 0.5f);
        glm::vec3 bot_right = spherical_to_cartesian(right_theta, bot_phi, 0.5f);

        make_tile(data, top_left, top_right, bot_left, bot_right);
    }
}

// Generates mesh for this primitive and binds VAOs and VBOs to it
// Don't call this function repeatedly. It will generate a new mesh each time.
// Call ONCE to generate a new mesh for all spheres (when primitive params change)
void Sphere::generate_mesh(const int shape_param_1, const int shape_param_2) {
    // Convert shape params into necessary minimum space
    int new_param_1 = std::max(shape_param_1, 2);
    int new_param_2 = std::max(shape_param_2, 3);

    // Check if we actually need to update mesh
    if (shape_parameter_1 == new_param_1 && shape_parameter_2 == new_param_2) {
        return;
    }

    // Updates parameters to match mesh we are generating, incorporating minimums
    shape_parameter_1 = new_param_1;
    shape_parameter_2 = new_param_2;

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

    // Create the sphere by iterating over the wedges
    float d_theta = (2 * std::numbers::pi) / shape_parameter_2;
    for (int wedge = 0; wedge < shape_parameter_2; wedge++) {
        make_wedge(data, d_theta * wedge, d_theta * (wedge + 1), shape_parameter_1);
    }

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
GLuint Sphere::get_vao() {
    return m_vao;
}

// Function to retrieve VBO for this primitive class
// Must be overriden to return the correct value (for this subclass only)
GLuint Sphere::get_vbo() {
    return m_vbo;
}

// Function to retrieve number of triangles for this class
// Must be overriden to return the correct value (for this subclass only)
int Sphere::get_triangles() {
    return num_triangles;
}

// Initialize the static variables to something
GLuint Sphere::m_vao = 0;
GLuint Sphere::m_vbo = 0;
int Sphere::num_triangles = 0;
int Sphere::shape_parameter_1 = 0;
int Sphere::shape_parameter_2 = 0;
