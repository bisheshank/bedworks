#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "camera/camera.h"
#include "primitives/sphere.h"
#include "primitives/cube.h"
#include "primitives/cylinder.h"
#include "primitives/cone.h"
#include "utils/sceneparser.h"
#include "utils/shaderloader.h"
#include "utils/shader.h"
#include "meshes/model.h"

// Holds light data in a specific way for passing to the shader
struct Light {
    glm::vec4 color;
    glm::vec3 pos;
    glm::vec3 attenuation_func;
    glm::vec4 dir;
    float penumbra;
    float angle;
    // Type indicates what kind of light this is
    // 0 -> Directional
    // 1 -> Point
    // 2 -> Spot
    // -1 -> Light doesn't exist
    int type = -1;
};

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void generate_scene();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void updateMeshes();
    void deleteMeshes();
    void send_light_to_shader(Shader shader, int index);
    void make_fbo();
    void delete_fbo();
    void paint_scene_geometry();
    void paint_model_geometry();
    void paint_post_process(GLuint texture);

    // Generate a rotation matrix using Rodrigues's rotation formula (very poggers)
    glm::mat3 generate_rotation_matrix(glm::vec3 axis, float radians);

    // Helper functions for rotations done via quaternions
    glm::vec4 quaternion_multiply(glm::vec4 q1, glm::vec4 q2);
    // The really important one that will actually output your rotated axis
    glm::vec3 quaternion_rotate(glm::vec3 to_rotate, glm::vec4 rotation);
    // Computes conjugate for quaternions
    glm::vec4 quaternion_conjugate(glm::vec4 quaternion);
    // Turns a rotation into a quaternion
    glm::vec4 rotation_to_quaternion(glm::vec3 axis, float theta);

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // Primitives of each type (used to generate the meshes)
    Sphere default_sphere;
    Cube default_cube;
    Cylinder default_cylinder;
    Cone default_cone;

    // Space to hold camera
    Camera m_camera;

    // Space to hold lights
    Light lights[8];

    // Space to hold primitives, separated by type
    std::vector<Sphere> spheres;
    std::vector<Cube> cubes;
    std::vector<Cylinder> cylinders;
    std::vector<Cone> cones;

    // Shaders for Phong lighting equation and framebuffer operations
    Shader m_phong_shader;
    Shader m_framebuffer_shader;
    Shader m_model_shader;
    Shader m_instancing_shader;

    // Global data
    float ka; // Ambient term
    float kd; // Diffuse term
    float ks; // Specular term

    // Parameters for changing
    int shape_param_1;
    int shape_param_2;

    // Params for view plane
    float near_plane;
    float far_plane;

    // Boolean indicating if we've started OpenGL stuff
    // Used to prevent trying to generate meshes before OpenGL initialized
    bool gl_initialized;

    // Conversion factor to determine conversion from mouse movement to radians
    float radian_conversion = std::numbers::pi / (360.0f * 2);

    // OpenGL member variables for rendering a framebuffer
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_fbo;

    // Member variables that stores screen size (required to implement the framebuffer)
    int m_fbo_height;
    int m_fbo_width;

    // Controls how much we convolve by (blur filter)
    int filter_radius = 5;

    // Default FBO counter (the one that actually displays stuff lol)
    GLuint default_fbo = 2;

    // For asteroid generation
    std::vector<glm::mat4> generateAsteroidTransformations(const unsigned int number = 2000);

    // For holding different models to instantiate
    // TODO: Possibly make this a vector with multiple planets
    Model planet;
    Model asteroids;
};
