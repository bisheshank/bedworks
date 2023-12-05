#include <stdexcept>
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

// Constructor
Camera::Camera() {
    // Basically an empty constructor
    // Random default values for all the fields
    m_look = glm::vec4(0.0, 0.0, -1.0, 0.0);
    m_up = glm::vec4(0.0, 1.0, 0.0, 0.0);
    m_pos = glm::vec4(0.0, 0.0, 0.0, 0.0);
    m_left = glm::vec4(0.0, 0.0, 0.0, 1.0);
    m_right = glm::vec4(0.0, 0.0, 0.0, -1.0);

    m_height_angle = M_PI / 2;
    m_far = 10.0f;
    m_near = 0.1f;
    m_aspect_ratio = 1.0f;

    m_rotate = glm::mat4(1.0f);
    m_inverse_rotate = glm::mat4(1.0f);
    m_translate = glm::mat4(1.0f);
    m_inverse_translate = glm::mat4(1.0f);
    m_view = glm::mat4(1.0f);
    m_proj = glm::mat4(1.0f);
    m_inverse_view = glm::mat4(1.0f);
}

// Recommended Constructor
Camera::Camera(SceneCameraData data, float new_near, float new_far, float aspect_ratio) {
    // Extract view-defining vectors from camera data
    m_look = data.look;
    m_up = data.up;

    // Extract projection defining constants from the other inputs (well, and the height angle)
    m_height_angle = data.heightAngle;
    m_far = new_far;
    m_near = new_near;
    m_aspect_ratio = aspect_ratio;

    // Generate submatrices for rotation and translation
    update_translation_matrix(data.pos);
    update_rotation_matrix(data.look, data.up);

    // Now use the submatrices!
    update_view_matrix();
    generate_projection_matrix();
}

// Applies update to the translation matrix and generates a new view matrix
void Camera::update_translation_matrix(glm::vec4 new_position) {
    // Update position field
    m_pos = new_position;

    // Update matrices (and inverses) to match
    m_translate = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -m_pos[0], -m_pos[1], -m_pos[2], 1.0f);
    m_inverse_translate = glm::inverse(m_translate);
}

// Applies update to the rotation matrix
void Camera::update_rotation_matrix(glm::vec4 new_look, glm::vec4 new_up) {
    // Convert fields to vec3s (lol)
    glm::vec3 look(new_look);
    look = normalize(look);
    glm::vec3 up(new_up);
    up = normalize(up);

    m_look = glm::vec4(normalize(look), 0.0f);
    m_up = glm::vec4(normalize(up), 0.0f);
    m_right = glm::normalize(glm::vec4(glm::cross(look, up), 0.0f));
    m_left = -m_right;

    // First compute w, v, and u
    glm::vec3 w = -look;
    glm::vec3 v = glm::normalize(up - ((glm::dot(up, w)) * w));
    glm::vec3 u = glm::cross(v, w);

    // Use this to compute Rotation matrix R
    m_rotate = glm::mat4(u[0], v[0], w[0], 0.0f, u[1], v[1], w[1], 0.0f, u[2], v[2], w[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    m_inverse_rotate = glm::inverse(m_rotate);
}

// Updates the view matrix to match translation and rotation matrices
void Camera::update_view_matrix() {
    // Combine stored rotation and translation matrices into view
    m_view = m_rotate * m_translate;
    // Also store the inverse (make sure this order is correct)
    m_inverse_view = m_inverse_rotate * m_inverse_translate;
}

// Generate projection matrix using fields and saves them
void Camera::generate_projection_matrix() {
    // Compute the matrix to translate the frustum into canonical frustum space
    float height_ratio = std::tan(m_height_angle / 2.0f);
    float width_ratio = height_ratio * m_aspect_ratio;
    glm::mat4 scale(1.0f / (m_far * width_ratio), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (m_far * height_ratio), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / m_far, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    // Compute the ratio of near and far plane, then initialize the perspective matrix
    float c = -m_near / m_far;
    glm::mat4 proj(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (1.0f + c), -1.0f, 0.0f, 0.0f, -c / (1.0f + c), 0.0f);

    // Now compute the matrix that translates to OpenGL space
    glm::mat4 gl_space(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f);

    // The result of the multiplication is the projection matrix
    m_proj = gl_space * proj * scale;
}

// Returns the view matrix for the current camera settings.
glm::mat4 Camera::get_view_matrix() {
    return m_view;
}

// Returns the inverse of the view matrix
glm::mat4 Camera::get_inverse_view_matrix() {
    return m_inverse_view;
}

// Returns the projection matrix
glm::mat4 Camera::get_projection_matrix() {
    return m_proj;
}

// Getter for camera position
glm::vec4 Camera::get_camera_pos() {
    return m_pos;
}

// Setter for camera position
void Camera::set_camera_pos(glm::vec4 new_pos) {
    m_pos = new_pos;
}

// Getter for look vector
glm::vec4 Camera::get_camera_look() {
    return m_look;
}

// Setter for look vector
void Camera::set_camera_look(glm::vec4 new_look) {
    m_look = new_look;
}

// Getter for up vector
glm::vec4 Camera::get_camera_up() {
    return m_up;
}

// Setter for up vector
void Camera::set_camera_up(glm::vec4 new_up) {
    m_up = new_up;
}

// Getter for near view plane
float Camera::get_camera_near() {
    return m_near;
}

// Setter for near view plane
void Camera::set_camera_near(float new_near) {
    m_near = new_near;
}

// Getter for far view plane
float Camera::get_camera_far() {
    return m_far;
}

// Setter for far view plane
void Camera::set_camera_far(float new_far) {
    m_far = new_far;
}

// Getter for aspect ratio
float Camera::get_camera_aspect_ratio() {
    return m_aspect_ratio;
}

// Setter for aspect ratio
void Camera::set_camera_aspect_ratio(float new_aspect_ratio) {
    m_aspect_ratio = new_aspect_ratio;
}

// Getter for height angle
float Camera::get_camera_height_angle() {
    return m_height_angle;
}

// Setter for height angle
void Camera::set_camera_height_angle(float new_height_angle) {
    m_height_angle = new_height_angle;
}

// Getter for left axis of translation
glm::vec4 Camera::get_left() {
    return m_left;
}

// Getter for right axis of translation
glm::vec4 Camera::get_right() {
    return m_right;
}
