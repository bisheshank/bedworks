#pragma once

#include <glm/glm.hpp>
#include "../utils/scenedata.h"
#include <tuple>

// A class representing a virtual camera.

class Camera {
public:
    // Constructors
    Camera();
    Camera(SceneCameraData data, float near, float far, float aspect_ratio);

    // Generates view matrix using fields and saves them
    void update_view_matrix();

    // Generate projection matrix using fields and saves them
    void generate_projection_matrix();

    // Returns the view matrix for the current camera settings.
    glm::mat4 get_view_matrix();

    // Returns the inverse of the view matrix
    glm::mat4 get_inverse_view_matrix();

    // Returns the projection matrix
    glm::mat4 get_projection_matrix();

    // Updates submatrices for viewing
    void update_translation_matrix(glm::vec4 new_position);
    void update_rotation_matrix(glm::vec4 new_look, glm::vec4 new_up);

    // Getter and setter for camera position
    glm::vec4 get_camera_pos();
    void set_camera_pos(glm::vec4 new_pos);

    // Getter and setter for look vector
    glm::vec4 get_camera_look();
    void set_camera_look(glm::vec4 new_look);

    // Getter and setter for up vector
    glm::vec4 get_camera_up();
    void set_camera_up(glm::vec4 new_up);

    // Getter and setter for near view plane
    float get_camera_near();
    void set_camera_near(float new_near);

    // Getter and setter for far view plane
    float get_camera_far();
    void set_camera_far(float new_far);

    // Getter and setter for aspect ratio
    float get_camera_aspect_ratio();
    void set_camera_aspect_ratio(float new_aspect_ratio);

    // Getter and setter for height angle
    float get_camera_height_angle();
    void set_camera_height_angle(float new_height_angle);

    // Getters for left and right axes of translation
    glm::vec4 get_left();
    glm::vec4 get_right();

private:
    // Vectors that define the camera in world space
    // TODO (Project 6): Modify these
    glm::vec4 m_pos;
    glm::vec4 m_look;
    glm::vec4 m_up;

    // Vector that describes the axis of translation orthogonal to look and up
    glm::vec4 m_left;
    glm::vec4 m_right;

    // Matrices that store rotational and translational operations for this camera
    glm::mat4 m_rotate;
    glm::mat4 m_translate;
    // Inverses
    glm::mat4 m_inverse_rotate;
    glm::mat4 m_inverse_translate;

    // Defines the near/far plane (projection matrix)
    float m_near;
    float m_far;

    // Height angle for camera (I think this is hard coded via config file)
    float m_height_angle;

    // Aspect ratio for camera (Compute this when the camera is initialized)
    // Aspect ratio is WIDTH / HEIGHT (this is important)
    float m_aspect_ratio;

    // View matrix (World space to camera space)
    glm::mat4 m_view;

    // Inverse of camera view matrix (camera space to world space)
    glm::mat4 m_inverse_view;

    // Projection matrix
    glm::mat4 m_proj;
};
