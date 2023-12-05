#include "cone.h"

// Constructor (does nothing different from base class)
Cone::Cone(const glm::mat4 &pctm, const ScenePrimitive &pprimitive) : Primitive(pctm, pprimitive)
{
    // Do nothing (nothing else needed to construct)
}

// Constructor (does nothing different from base class)
Cone::Cone() : Primitive()
{
    // Do nothing (nothing else needed to construct)
}

// Destructor
Cone::~Cone() {
    // Then if there are no more active Cylinders, delete the VAOs and VBOs
}

// Function to draw the cone
void Cone::draw(GLuint shader) {
    // If there aren't enough wedges, don't render this shape
    if (shape_parameter_2 < 3 || shape_parameter_1 < 1) {
        return;
    }

    Primitive::draw(shader);
}

// Function to delete buffers
void Cone::delete_buffers() {
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

// Make a wedge for the cone
void Cone::make_wedge(std::vector<GLfloat> &data, float curr_theta, float next_theta) {
    // Make the cone top
    float delta_h = -1.0f / shape_parameter_1;
    float delta_r = 0.5f / shape_parameter_1;

    // Top vertex of upper triangle
    glm::vec3 top = cylinder_to_cartesian(0, 0, 0.5f);
    // Bottom left vertex of upper triangle
    glm::vec3 bot_left = cylinder_to_cartesian(curr_theta, delta_r, 0.5f + delta_h);
    // Bottom right vertex of upper triangle
    glm::vec3 bot_right = cylinder_to_cartesian(next_theta, delta_r, 0.5f + delta_h);

    // Insert top vertex
    insert_vec3(data, top);
    // Normal (we use a modified version)
    insert_vec3(data, compute_normal(cylinder_to_cartesian((curr_theta + next_theta) / 2.0f, delta_r, 0.5f + delta_h)));
    // ALT: Uses cross product
    // insert_vec3(data, glm::cross(bot_left - top, bot_right - top));

    // Insert the other two vertices
    insert_vec3(data, bot_left);
    insert_vec3(data, compute_normal(bot_left));
    insert_vec3(data, bot_right);
    insert_vec3(data, compute_normal(bot_right));

    num_triangles += 1;

    // Construct the faces along the cone
    for (int slices = 1; slices < shape_parameter_1; slices++) {
        // Generate a tile counterclockwise
        glm::vec3 top_left = cylinder_to_cartesian(curr_theta, delta_r * slices, 0.5f + (delta_h * slices));
        glm::vec3 top_right = cylinder_to_cartesian(next_theta, delta_r * slices, 0.5f + (delta_h * slices));
        glm::vec3 bottom_left = cylinder_to_cartesian(curr_theta, delta_r * (slices + 1), 0.5f + (delta_h * (slices + 1)));
        glm::vec3 bottom_right = cylinder_to_cartesian(next_theta, delta_r * (slices + 1), 0.5f + (delta_h * (slices + 1)));

        make_side_tile(data, top_left, top_right, bottom_left, bottom_right);
    }

    // Make the bottom of the wedge
    // Create the first triangle
    insert_vec3(data, cylinder_to_cartesian(0, 0, -0.5f));
    insert_vec3(data, glm::vec3(0.0f, -1.0f, 0.0f));
    insert_vec3(data, cylinder_to_cartesian(next_theta, delta_r, -0.5f));
    insert_vec3(data, glm::vec3(0.0f, -1.0f, 0.0f));
    insert_vec3(data, cylinder_to_cartesian(curr_theta, delta_r, -0.5f));
    insert_vec3(data, glm::vec3(0.0f, -1.0f, 0.0f));

    num_triangles += 1;

    // Create the rest of the slices (tiled)
    for (int slices = 1; slices < shape_parameter_1; slices++) {
        // Generate the tile counterclockwise
        glm::vec3 top_left = cylinder_to_cartesian(curr_theta, delta_r * (slices + 1), -0.5f);
        glm::vec3 top_right = cylinder_to_cartesian(next_theta, delta_r * (slices + 1), -0.5f);
        glm::vec3 bottom_left = cylinder_to_cartesian(curr_theta, delta_r * slices, -0.5f);
        glm::vec3 bottom_right = cylinder_to_cartesian(next_theta, delta_r * slices, -0.5f);

        // STICK IT IN THAT BAD BOY
        make_cap_tile(data, top_left, top_right, bottom_left, bottom_right);
    }
}

// Make a tile for the cap of the cone (the bottom bit)
void Cone::make_cap_tile(std::vector<GLfloat> &data, glm::vec3 top_left,
                         glm::vec3 top_right,
                         glm::vec3 bottom_left,
                         glm::vec3 bottom_right) {
    // Identical to the cube make_tile
    // Normal can be computed by normalizing CB - CA
    // (1st - 3rd) X (2nd - 3rd)
    // This normal is the same (I think) for both triangles
    // (bottomLeft - bottomRight) X (topLeft - bottomRight)
    glm::vec3 normal = glm::normalize(glm::cross((top_left - bottom_right), (bottom_left - bottom_right)));

    // First triangle
    insert_vec3(data, top_left);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_left);
    insert_vec3(data, normal);
    insert_vec3(data, top_right);
    insert_vec3(data, normal);

    num_triangles += 1;

    // Second triangle
    insert_vec3(data, top_right);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_left);
    insert_vec3(data, normal);
    insert_vec3(data, bottom_right);
    insert_vec3(data, normal);

    num_triangles += 1;
}

// Make a side tile for the cone
void Cone::make_side_tile(std::vector<GLfloat> &data, glm::vec3 top_left,
                          glm::vec3 top_right,
                          glm::vec3 bottom_left,
                          glm::vec3 bottom_right) {
    // First triangle
    insert_vec3(data, top_left);
    insert_vec3(data, compute_normal(top_left));
    insert_vec3(data, bottom_left);
    insert_vec3(data, compute_normal(bottom_left));
    insert_vec3(data, top_right);
    insert_vec3(data, compute_normal(top_right));

    num_triangles += 1;

    // Second triangle
    insert_vec3(data, top_right);
    insert_vec3(data, compute_normal(top_right));
    insert_vec3(data, bottom_left);
    insert_vec3(data, compute_normal(bottom_left));
    insert_vec3(data, bottom_right);
    insert_vec3(data, compute_normal(bottom_right));

    num_triangles += 1;
}

// Function to compute normal vector
glm::vec3 Cone::compute_normal(glm::vec3 point) {
    // Compute the radius of the cone at this point
    float radius = (0.5f - point[1]) / 2.0f;
    // Return the normal according to this formula
    return glm::normalize(glm::vec3(2 * point[0], radius, 2 * point[2]));
}

// Generates mesh for this primitive and binds VAOs and VBOs to it
// Don't call this function repeatedly. It will generate a new mesh each time.
// Call ONCE to generate a new mesh for all Cones (when primitive params change)
void Cone::generate_mesh(const int shape_param_1, const int shape_param_2) {
    // Clip parameters to appropriate range
    int new_param_1 = std::max(shape_param_1, 1);
    int new_param_2 = std::max(shape_param_2, 3);

    // Only generate mesh if parameters changed
    if (shape_parameter_1 == new_param_1 && shape_parameter_2 == new_param_2) {
        return;
    }

    // Update the shape parameters
    shape_parameter_1 = new_param_1;
    shape_parameter_2 = new_param_2;

    // Check if the shape is renderable
    if (shape_parameter_2 < 3 || shape_parameter_1 < 1) {
        return;
    }

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

    // Create the Cylinder by iterating over the wedges
    float d_theta = (2 * std::numbers::pi) / shape_parameter_2;
    for (int wedge = 0; wedge < shape_parameter_2; wedge++) {
        make_wedge(data, d_theta * wedge, d_theta * (wedge + 1));
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
GLuint Cone::get_vao() {
    return m_vao;
}

// Function to retrieve VBO for this primitive class
// Must be overriden to return the correct value (for this subclass only)
GLuint Cone::get_vbo() {
    return m_vbo;
}

// Function to retrieve number of triangles for this class
// Must be overriden to return the correct value (for this subclass only)
int Cone::get_triangles() {
    return num_triangles;
}

// Initialize the static variables to something
GLuint Cone::m_vao = 0;
GLuint Cone::m_vbo = 0;
int Cone::num_triangles = 0;
int Cone::shape_parameter_1 = 0;
int Cone::shape_parameter_2 = 0;
