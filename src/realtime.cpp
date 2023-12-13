// DON'T TOUCH THIS DEFINE, PLEASE
#define STB_IMAGE_IMPLEMENTATION
#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/gtc/quaternion.hpp"
#include "settings.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include "noise/fastnoise.h"
#include <glm/gtx/string_cast.hpp>

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // TIME TO INITIALIZE ALL OUR FIELDS!!!

    // Configure default primitive pointers
    default_sphere = Sphere();
    default_cube = Cube();
    default_cylinder = Cylinder();
    default_cone = Cone();

    // Configure lights (disable them all)
    for (int i = 0; i < 8; i++) {
        lights[i].type = -1;
    }

    // Initialize global data
    ka = 0;
    kd = 0;
    ks = 0;

    // Initialize shape params
    shape_param_1 = 5;
    shape_param_2 = 5;

    // Initialize far and near planes
    near_plane = 0.1f;
    far_plane = 10.f;

    // Set gl_initialized to false because we haven't set up yet
    gl_initialized = false;

    // Set all the GL fields to 0
    m_fullscreen_vbo = 0;
    m_fullscreen_vao = 0;
    m_fbo_texture = 0;
    m_fbo_renderbuffer = 0;
    m_fbo = 0;

    // SHADERS!
    m_phong_shader = Shader();
    m_framebuffer_shader = Shader();
    m_model_shader = Shader();
    m_instancing_shader = Shader();
    m_skybox_shader = Shader();

    // MODELS!
    planet = Model();
    asteroids = Model();

    // SKYBOX!
    box = Skybox();
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    deleteMeshes();

    // Cleanup framebuffer memory
    delete_fbo();

    // Free the fullscreen VAO and VBO (which supplies texture mapping data)
    if (m_fullscreen_vao != 0) {
        glDeleteVertexArrays(1, &m_fullscreen_vao);
        Debug::glErrorCheck();
    }
    // Only delete this data if we created it in the first place
    if (m_fullscreen_vbo != 0) {
        glDeleteBuffers(1, &m_fullscreen_vbo);
        Debug::glErrorCheck();
    }

    // Clean up all associated model data here
    planet.cleanup();
    asteroids.cleanup();

    // Cleanup all shader stuff here
    m_phong_shader.Delete();
    m_framebuffer_shader.Delete();
    m_model_shader.Delete();
    m_instancing_shader.Delete();
    m_skybox_shader.Delete();

    this->doneCurrent();
}

// Function to cleanup OpenGL memory associated with a framebuffer
void Realtime::delete_fbo() {
    // Delete texture, renderbuffer, and framebuffer
    if (m_fbo_texture != 0) {
        glDeleteTextures(1, &m_fbo_texture);
        Debug::glErrorCheck();
    }
    // Use conditionals to not make extra delete calls
    if (m_fbo_renderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
        Debug::glErrorCheck();
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        Debug::glErrorCheck();
    }
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    // Now we can determine how big our FBOs and screen should be
    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    // Like loading the shader
    m_phong_shader.loadData(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");
    // And the other shader
    m_framebuffer_shader.loadData(":/resources/shaders/framebuffer.vert", ":/resources/shaders/framebuffer.frag");
    m_model_shader.loadData(":/resources/shaders/model.vert", ":/resources/shaders/model.frag");
    m_instancing_shader.loadData(":/resources/shaders/instancing.vert", ":/resources/shaders/model.frag");
    m_skybox_shader.loadData(":/resources/shaders/skybox.vert", ":/resources/shaders/skybox.frag");

    // The skybox shouldn't change when loading a new scene (only where the model is, so we can load it here)
    // Note the order for the elements of the Skybox must be in:
    // RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK
    // According to this pattern: https://learnopengl.com/img/advanced/cubemaps_skybox.png
    std::string working_dir = QDir::currentPath().toStdString();
    std::string path_to_skybox_dir = "/resources/skybox/";

    // Using 6 of the same images for now (testing)
    std::vector<std::string> skybox_images = {
        working_dir + path_to_skybox_dir + "right.jpg",
        working_dir + path_to_skybox_dir + "left.jpg",
        working_dir + path_to_skybox_dir + "top.jpg",
        working_dir + path_to_skybox_dir + "bottom.jpg",
        working_dir + path_to_skybox_dir + "front.jpg",
        working_dir + path_to_skybox_dir + "back.jpg"
    };

    // When the program starts, the skybox should be loaded
    box.load_texture(skybox_images);

    // Now that we've initialized GL, we can actually process settings changes
    gl_initialized = true;
    updateMeshes();

    // Create data that encodes the two triangles that make up a framebuffer
    // Allows for mapping over our rendered image as if it is a texture
    std::vector<GLfloat> fullscreen_quad_data =
    { // Positions, followed by texture mapping (UV) coordinates
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f
    };

    // Create the necessary OpenGL objects
    // Note we don't need to regenerate this data since our UV mapping strategy for the screen never changes

    // VBO object for this FBO
    glGenBuffers(1, &m_fullscreen_vbo);
    Debug::glErrorCheck();
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    Debug::glErrorCheck();

    // VAO object for this FBO
    glGenVertexArrays(1, &m_fullscreen_vao);
    Debug::glErrorCheck();
    glBindVertexArray(m_fullscreen_vao);
    Debug::glErrorCheck();

    // Load the fullscreen data into the FBO VBO (the buffer)
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size() * sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();

    // Initialize the VAO with data in the VBO
    // Positions on screen
    glEnableVertexAttribArray(0);
    Debug::glErrorCheck();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    Debug::glErrorCheck();

    // UV coordinates for texture mapping
    glEnableVertexAttribArray(1);
    Debug::glErrorCheck();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    Debug::glErrorCheck();

    // Reset OpenGL state (by unbinding the VAOs and VBOs)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Debug::glErrorCheck();
    glBindVertexArray(0);
    Debug::glErrorCheck();

    // Make the FBO
    make_fbo();
}

// Function to make the framebuffer (lifted straight from lab)
// Basically generates a new texture and renderbuffer to write output to and then display to user
void Realtime::make_fbo() {
    // Cleanup old FBO data
    delete_fbo();

    // Generate a new OpenGL texture and configure the UV sampling mechanism to linear interpolation
    glGenTextures(1, &m_fbo_texture);
    Debug::glErrorCheck();
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    Debug::glErrorCheck();

    // Initialize an empty texture
    // Note that texture size is configured to the size of the FBO
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    Debug::glErrorCheck();

    // Set to linear interpolation for UV sampling (we don't have very many triangles)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Debug::glErrorCheck();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Debug::glErrorCheck();

    // We've configured the texture for the FBO, unbind it
    glBindTexture(GL_TEXTURE_2D, 0);
    Debug::glErrorCheck();

    // Generate a renderbuffer (this is where we store what's actually displayed to the user)
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    Debug::glErrorCheck();
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    Debug::glErrorCheck();
    // Configure appropriate amount of space for everything we want
    // Note that this space is configured to the size of the FBO
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    Debug::glErrorCheck();

    // And unbind when done
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Debug::glErrorCheck();

    // Now create a framebuffer (which has both a texture and a renderbuffer, like how a VAO is attached to a VBO)
    glGenFramebuffers(1, &m_fbo);
    Debug::glErrorCheck();
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    Debug::glErrorCheck();

    // Attach the texture and renderbuffer to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    Debug::glErrorCheck();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);
    Debug::glErrorCheck();

    // Reset OpenGL state by unbinding framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Debug::glErrorCheck();
}

// Students: anything requiring OpenGL calls every frame should be done here
void Realtime::paintGL() {
    // Try to avoid divide by 0s if the near/far viewplanes overlap
    if (near_plane == far_plane) {
        return;
    }

    // Render our scene to the framebuffer first
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    Debug::glErrorCheck();

    // Configure viewport to the framebuffer
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    Debug::glErrorCheck();

    // Clear framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Debug::glErrorCheck();

    // Now paint the scene geometry
    paint_scene_geometry();

    // The really neat stuff we actually care about!!!
    paint_model_geometry();

    // The moment of truth. Paint the skybox...
    paint_skybox();

    // Render scene to the default framebuffer (the one that we actually display our stuff on)
    glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    Debug::glErrorCheck();
    // glViewport(0, 0, m_fbo_width * this->devicePixelRatio(), m_fbo_height * this->devicePixelRatio());

    // Clear the default FBO before we draw to it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Debug::glErrorCheck();

    // Helper to apply post processing
    paint_post_process(m_fbo_texture);
}

// Function to make the skybox (similar to making the model)
void Realtime::paint_skybox() {
    m_skybox_shader.Activate();

    // Compute a version of the view matrix that doesn't use translation (do not want to translate skybox)
    glm::mat4 view_no_translate = glm::mat4(glm::mat3(m_camera.get_view_matrix()));

    // Send necessary uniforms for camera
    GLuint location;
    location = glGetUniformLocation(m_skybox_shader.ID, "view_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &view_no_translate[0][0]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_skybox_shader.ID, "proj_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_projection_matrix()))[0][0]);
    Debug::glErrorCheck();

    // DRAW THE BOX
    box.draw(m_skybox_shader);

    m_skybox_shader.Deactivate();
}

// New func to test painting model shaders
void Realtime::paint_model_geometry() {
    m_model_shader.Activate();

    // Send necessary uniforms for camera
    GLuint location;
    location = glGetUniformLocation(m_model_shader.ID, "view_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_view_matrix())[0][0]));
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_model_shader.ID, "proj_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_projection_matrix()))[0][0]);
    Debug::glErrorCheck();

    // Remaining uniforms for vertex shader sent via model

    // Uniform for light needs to be sent via this func, other samplers are sent via model
    // Camera position
    // TODO: Change to actually load in light data like the original Phong Shader
    location = glGetUniformLocation(m_model_shader.ID, "camera_pos");
    Debug::glErrorCheck();
    glUniform3f(location, m_camera.get_camera_pos()[0], m_camera.get_camera_pos()[1], m_camera.get_camera_pos()[2]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_model_shader.ID, "light_color");
    Debug::glErrorCheck();
    glUniform4f(location, 1.0f, 1.0f, 1.0f, 1.0f);
    Debug::glErrorCheck();
    location = glGetUniformLocation(m_model_shader.ID, "light_pos");
    Debug::glErrorCheck();
    glUniform3f(location, 0.0f, 0.0f, 0.0f);
    Debug::glErrorCheck();

    // Draw planet using model shader (since it is not instanced
    planet.Draw(m_model_shader);
    m_model_shader.Deactivate();

    // Asteroids need to be configured to use separate model shader

    m_instancing_shader.Activate();

    // Send necessary uniforms for the instancing shader
    location = glGetUniformLocation(m_instancing_shader.ID, "view_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_view_matrix())[0][0]));
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_instancing_shader.ID, "proj_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_projection_matrix()))[0][0]);
    Debug::glErrorCheck();

    // Uniform for light needs to be sent via this func, other samplers are sent via model
    // Camera position
    // TODO: Change to actually load in light data like the original Phong Shader
    location = glGetUniformLocation(m_instancing_shader.ID, "camera_pos");
    Debug::glErrorCheck();
    glUniform3f(location, m_camera.get_camera_pos()[0], m_camera.get_camera_pos()[1], m_camera.get_camera_pos()[2]);
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_instancing_shader.ID, "light_color");
    Debug::glErrorCheck();
    glUniform4f(location, 1.0f, 1.0f, 1.0f, 1.0f);
    Debug::glErrorCheck();
    location = glGetUniformLocation(m_instancing_shader.ID, "light_pos");
    Debug::glErrorCheck();
    glUniform3f(location, 0.0f, 0.0f, 0.0f);
    Debug::glErrorCheck();

    asteroids.Draw(m_instancing_shader);

    m_instancing_shader.Deactivate();
}

// Helper function to apply post processing effects to rendered image
void Realtime::paint_post_process(GLuint texture) {
    // Using the framebuffer shader for postprocessing
    m_framebuffer_shader.Activate();

    // Bind the necessary OpenGL components
    // Fullscreen VAO (the thing that defines the UV coordinates)
    glBindVertexArray(m_fullscreen_vao);
    Debug::glErrorCheck();

    // Activate and bind texture we want (framebuffer)
    glActiveTexture(GL_TEXTURE0);
    Debug::glErrorCheck();
    glBindTexture(GL_TEXTURE_2D, texture);
    Debug::glErrorCheck();

    // Send necessary uniforms to the framebuffer shader
    GLuint location;

    // Send texture so we can sample from it
    location = glGetUniformLocation(m_framebuffer_shader.ID, "sampler2D");
    Debug::glErrorCheck();
    glUniform1i(location, GL_TEXTURE0);
    Debug::glErrorCheck();

    // Send booleans indicating if we should use certain post-processing techniques
    // Per-pixel
    location = glGetUniformLocation(m_framebuffer_shader.ID, "per_pixel");
    Debug::glErrorCheck();
    glUniform1i(location, settings.perPixelFilter);
    Debug::glErrorCheck();

    // Kernel-based
    location = glGetUniformLocation(m_framebuffer_shader.ID, "per_kernel");
    Debug::glErrorCheck();
    glUniform1i(location, settings.kernelBasedFilter);
    Debug::glErrorCheck();

    // Send the width of the screen for convolution
    location = glGetUniformLocation(m_framebuffer_shader.ID, "u_step");
    Debug::glErrorCheck();
    glUniform1f(location, 1.0f / (m_fbo_width * m_devicePixelRatio));
    Debug::glErrorCheck();

    // Send the height of the screen for convolution
    location = glGetUniformLocation(m_framebuffer_shader.ID, "v_step");
    Debug::glErrorCheck();
    glUniform1f(location, 1.0f / (m_fbo_height * m_devicePixelRatio));
    Debug::glErrorCheck();

    // Send radius of convolution
    location = glGetUniformLocation(m_framebuffer_shader.ID, "radius");
    Debug::glErrorCheck();
    glUniform1i(location, filter_radius);
    Debug::glErrorCheck();

    // Draw!
    glDrawArrays(GL_TRIANGLES, 0, 6);
    Debug::glErrorCheck();

    // Reset default state
    glBindTexture(GL_TEXTURE_2D, 0);
    Debug::glErrorCheck();
    glBindVertexArray(0);
    Debug::glErrorCheck();
    glUseProgram(0);
    Debug::glErrorCheck();
}

// Helper function that paints the scene geometry to whatever framebuffer we want to paint to (default or our own)
void Realtime::paint_scene_geometry() {
    // Normally one would iterate over shaders, but since we only have 1 shader that's not necessary
    m_phong_shader.Activate();
    Debug::glErrorCheck();

    // Pass in relevant uniforms defined in the Realtime instance (Camera, lights)
    GLuint location;
    location = glGetUniformLocation(m_phong_shader.ID, "view_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_view_matrix())[0][0]));
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_phong_shader.ID, "proj_matrix");
    Debug::glErrorCheck();
    glUniformMatrix4fv(location, 1, GL_FALSE, &((m_camera.get_projection_matrix()))[0][0]);
    Debug::glErrorCheck();

    // Pass in ALL the uniforms required for the shader (Lights)
    for(int i = 0; i < 8; i++) {
        // Compute the location for inserting the light
        send_light_to_shader(m_phong_shader, i);
    }

    // Pass in the global data uniforms
    location = glGetUniformLocation(m_phong_shader.ID, "ka");
    Debug::glErrorCheck();
    glUniform1f(location, ka);
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_phong_shader.ID, "kd");
    Debug::glErrorCheck();
    glUniform1f(location, kd);
    Debug::glErrorCheck();

    location = glGetUniformLocation(m_phong_shader.ID, "ks");
    Debug::glErrorCheck();
    glUniform1f(location, ks);
    Debug::glErrorCheck();

    // Camera position
    location = glGetUniformLocation(m_phong_shader.ID, "camera_pos");
    Debug::glErrorCheck();
    glUniform3f(location, m_camera.get_camera_pos()[0], m_camera.get_camera_pos()[1], m_camera.get_camera_pos()[2]);
    Debug::glErrorCheck();

    // Iterate over all primitives and render them using their draw function
    // Bind VAO for spheres when we draw them
    if (default_sphere.get_vao() != 0) {
        glBindVertexArray(default_sphere.get_vao());
        Debug::glErrorCheck();

        // Then draw them
        for(int i = 0; i < spheres.size(); i++) {
            spheres[i].draw(m_phong_shader.ID);
        }

        // Then unbind the spheres
        glBindVertexArray(0);
        Debug::glErrorCheck();
    }

    // We do each primitive type separately because (pending optimizations) you can bind a single VAO once
    // Bind VAO for cubes when we draw them
    if (default_cube.get_vao() != 0) {
        glBindVertexArray(default_cube.get_vao());
        Debug::glErrorCheck();

        // Then draw them
        for(int i = 0; i < cubes.size(); i++) {
            cubes[i].draw(m_phong_shader.ID);
        }

        // Then unbind the cubes
        glBindVertexArray(0);
        Debug::glErrorCheck();
    }

    // Bind VAO for cylinders when we draw them
    if (default_cylinder.get_vao() != 0) {
        glBindVertexArray(default_cylinder.get_vao());
        Debug::glErrorCheck();

        // Then draw them
        for(int i = 0; i < cylinders.size(); i++) {
            cylinders[i].draw(m_phong_shader.ID);
        }

        // Then unbind the cylinders
        glBindVertexArray(0);
        Debug::glErrorCheck();
    }

    // Bind VAO for cones when we draw them
    if (default_cone.get_vao() != 0) {
        glBindVertexArray(default_cone.get_vao());
        Debug::glErrorCheck();

        // Then draw them
        for(int i = 0; i < cones.size(); i++) {
            cones[i].draw(m_phong_shader.ID);
        }

        // Then unbind the cylinders
        glBindVertexArray(0);
        Debug::glErrorCheck();
    }

    glUseProgram(0);
    Debug::glErrorCheck();
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    // Compute and cache window size (to regenerate framebuffer)
    m_fbo_width = w * m_devicePixelRatio;
    m_fbo_height = h * m_devicePixelRatio;
    glViewport(0, 0, m_fbo_width, m_fbo_height);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    // Update the camera fields
    m_camera.set_camera_aspect_ratio((float) w / h);
    m_camera.generate_projection_matrix();

    // We've reconfigured the size of the screen, remake the FBO with this size
    make_fbo();

    // New draw!
    update();
}

// Function that generates a new allotment of asteroids
void Realtime::generate_scene() {
    makeCurrent();
    // Let's just focus on generating a new planet
    std::string working_dir = QDir::currentPath().toStdString();
    std::string planet_path = "/resources/models/planet/scene.gltf";

    std::cerr << "Trying to load planet model...\n";
    planet.loadModel((working_dir + planet_path).c_str());
    std::cerr << "Planet model loaded using path: " << working_dir << planet_path << "\n";

    // Also add asteroids
    // FIX some instancing number
    unsigned int instances = 2500;
    std::vector<glm::mat4> asteroid_matrices = generateAsteroidTransformations(instances);
    std::string asteroid_path = "/resources/models/asteroid/scene.gltf";

    std::cerr << "Trying to load " << instances << " instances of asteroids model...\n";
    asteroids.loadModel((working_dir + asteroid_path).c_str(), instances, asteroid_matrices);
    std::cerr << "Asteroid model loaded using path: " << working_dir << asteroid_path << "\n";
}

// Load a new scene file's data into the scene
void Realtime::sceneChanged() {
    makeCurrent();
    // Clear existing data
    // Remove all lights (disable their type)
    for (int i = 0; i < 8; i++) {
        lights[i].type = -1;
    }

    // Remove all primitives from lists to draw
    spheres.clear();
    cubes.clear();
    cylinders.clear();
    cones.clear();

    // This is where you load the scene and parse it
    RenderData data;
    SceneParser::parse(settings.sceneFilePath, data);

    // Store the scene's lights
    for (int i = 0; i < data.lights.size(); i++) {
        // Copy out the relevant data into our structs which get sent to the shader
        lights[i].color = data.lights[i].color;
        lights[i].pos = glm::vec3(data.lights[i].pos);
        lights[i].attenuation_func = data.lights[i].function;
        lights[i].dir = data.lights[i].dir;
        lights[i].penumbra = data.lights[i].penumbra;
        lights[i].angle = data.lights[i].angle;

        // Initialize light type
        switch(data.lights[i].type) {
            case(LightType::LIGHT_DIRECTIONAL):
                lights[i].type = 0;
                break;
            case(LightType::LIGHT_POINT):
                lights[i].type = 1;
                break;
            case(LightType::LIGHT_SPOT):
                lights[i].type = 2;
                break;
        }
    }

    // Store the scene's shapes
    for (int i = 0; i < data.shapes.size(); i++) {
        switch(data.shapes[i].primitive.type) {
            case(PrimitiveType::PRIMITIVE_CUBE):
                cubes.emplace_back(data.shapes[i].ctm, data.shapes[i].primitive);
                break;
            // Initialize sphere
            case(PrimitiveType::PRIMITIVE_SPHERE):
                spheres.emplace_back(data.shapes[i].ctm, data.shapes[i].primitive);
                break;
            // Initialize cylinder
            case(PrimitiveType::PRIMITIVE_CYLINDER):
                cylinders.emplace_back(data.shapes[i].ctm, data.shapes[i].primitive);
                break;
            // Initialize cone
            case(PrimitiveType::PRIMITIVE_CONE):
                cones.emplace_back(data.shapes[i].ctm, data.shapes[i].primitive);
                break;
            // Initialize mesh (well, we can't)
            case(PrimitiveType::PRIMITIVE_MESH):
                throw(std::runtime_error("Rendering mesh objects not supported. Exiting..."));
                break;
        }
    }

    // Update meshes
    updateMeshes();

    // Store the camera data
    float aspect_ratio = (float) size().width() / (float) size().height();
    m_camera = Camera(data.cameraData, settings.nearPlane, settings.farPlane, aspect_ratio);

    // Store the global data
    ka = data.globalData.ka;
    kd = data.globalData.kd;
    ks = data.globalData.ks;

    // Ask for a paint GL call
    update();
}

// Function to send a single light of data to shader
void Realtime::send_light_to_shader(Shader shader, int index) {
    // Prepare some variables to determine where we can put uniforms for this struct
    std::string light_index = "lights[" + std::to_string(index) + "]";
    GLuint location;

    // Send each of the fields to the shader
    std::string color_loc = light_index + ".color";
    location = glGetUniformLocation(shader.ID, color_loc.c_str());
    Debug::glErrorCheck();
    glUniform4f(location, lights[index].color[0], lights[index].color[1], lights[index].color[2], lights[index].color[3]);
    Debug::glErrorCheck();

    std::string pos_loc = light_index + ".pos";
    location = glGetUniformLocation(shader.ID, pos_loc.c_str());
    Debug::glErrorCheck();
    glUniform3f(location, lights[index].pos[0], lights[index].pos[1], lights[index].pos[2]);
    Debug::glErrorCheck();

    std::string attenuation_func_loc = light_index + ".attenuation_func";
    location = glGetUniformLocation(shader.ID, attenuation_func_loc.c_str());
    Debug::glErrorCheck();
    glUniform3f(location, lights[index].attenuation_func[0], lights[index].attenuation_func[1], lights[index].attenuation_func[2]);
    Debug::glErrorCheck();

    std::string dir_loc = light_index + ".dir";
    location = glGetUniformLocation(shader.ID, dir_loc.c_str());
    Debug::glErrorCheck();
    glUniform4f(location, lights[index].dir[0], lights[index].dir[1], lights[index].dir[2], lights[index].dir[3]);
    Debug::glErrorCheck();

    std::string penumbra_loc = light_index + ".penumbra";
    location = glGetUniformLocation(shader.ID, penumbra_loc.c_str());
    Debug::glErrorCheck();
    glUniform1f(location, lights[index].penumbra);
    Debug::glErrorCheck();

    std::string angle_loc = light_index + ".angle";
    location = glGetUniformLocation(shader.ID, angle_loc.c_str());
    Debug::glErrorCheck();
    glUniform1f(location, lights[index].angle);
    Debug::glErrorCheck();

    std::string type_loc = light_index + ".type";
    location = glGetUniformLocation(shader.ID, type_loc.c_str());
    Debug::glErrorCheck();
    glUniform1i(location, lights[index].type);
    Debug::glErrorCheck();
}

// Function to update all meshes in OpenGL
void Realtime::updateMeshes() {
    // Sphere
    default_sphere.generate_mesh(shape_param_1, shape_param_2);

    // Cube
    default_cube.generate_mesh(shape_param_1, shape_param_2);

    // Cylinder
    default_cylinder.generate_mesh(shape_param_1, shape_param_2);

    // Cone
    default_cone.generate_mesh(shape_param_1, shape_param_2);
}

// Function to delete the meshes of all primitives
void Realtime::deleteMeshes() {
    // Sphere
    default_sphere.delete_buffers();

    // Cube
    default_cube.delete_buffers();

    // Cylinder
    default_cylinder.delete_buffers();

    // Cone
    default_cone.delete_buffers();
}

void Realtime::settingsChanged() {
    // Don't do anything if we haven't been initialized
    if (!gl_initialized) {
        return;
    }

    // Check if changed occurred to shape parameters
    if (shape_param_1 != settings.shapeParameter1 || shape_param_2 != settings.shapeParameter2) {
        makeCurrent();

        // Regenerate meshes with the new shape parameters
        shape_param_1 = settings.shapeParameter1;
        shape_param_2 = settings.shapeParameter2;
        updateMeshes();
    }

    // Check if change occurred to view plane
    if (near_plane != settings.nearPlane || far_plane != settings.farPlane) {
        near_plane = settings.nearPlane;
        far_plane = settings.farPlane;

        // Make sure they don't overlap (that's wonky)
        if (near_plane != far_plane) {
            // Update camera fields, then generate new projection matrix
            m_camera.set_camera_near(near_plane);
            m_camera.set_camera_far(far_plane);
            m_camera.generate_projection_matrix();
        }
    }

    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

// Helper function to generate rotation matrices using Rodrigue's rotation formula
glm::mat3 Realtime::generate_rotation_matrix(glm::vec3 axis, float radians) {
    // Normalize axis if it isn't already
    axis = glm::normalize(axis);

    // Cache sine and cosine values for computation
    float sine = sin(radians);
    float cosine = cos(radians);

    // Generate the matrix
    glm::vec3 col_1(cosine + (axis[0] * axis[0] * (1.0f - cosine)), (axis[0] * axis[2] * (1.0f - cosine)) + (axis[1] * sine), (axis[0] * axis[1] * (1.0f - cosine)) - (axis[2] * sine));
    glm::vec3 col_2((axis[0] * axis[2] * (1.0f - cosine)) - (axis[1] * sine), cosine + (axis[2] * axis[2] * (1.0f - cosine)), (axis[2] * axis[1] * (1.0f - cosine)) - (axis[0] * sine));
    glm::vec3 col_3((axis[0] * axis[1] * (1.0f - cosine)) + (axis[2] * sine), (axis[2] * axis[1] * (1.0f - cosine)) - (axis[0] * sine), cosine + (axis[1] * axis[1] * (1.0f - cosine)));

    return glm::mat3(col_1, col_2, col_3);
}

// Helper functions for rotations done via quaternions
glm::vec4 Realtime::quaternion_multiply(glm::vec4 q1, glm::vec4 q2) {
    // Compute the real component
    float real = q1[3] * q2[3] - (glm::dot(glm::vec3(q2), glm::vec3(q1)));
    // Compute the imaginary component
    glm::vec3 imag = (glm::vec3(q2) * q1[3]) + (glm::vec3(q1) * q2[3]) + glm::cross(glm::vec3(q1), glm::vec3(q2));

    // Construct quaternion
    return glm::vec4(imag, real);
}

// The really important one that will actually output your rotated axis
glm::vec3 Realtime::quaternion_rotate(glm::vec3 to_rotate, glm::vec4 rotation) {
    // Turn the vector to rotate into a quaternion
    glm::vec4 quat_to_rotate(to_rotate, 0.0f);

    // Compute the rotation
    return glm::vec3(quaternion_multiply(quaternion_multiply(rotation, quat_to_rotate), quaternion_conjugate(rotation)));
}

// Computes conjugate for quaternions
glm::vec4 Realtime::quaternion_conjugate(glm::vec4 quaternion) {
    return glm::vec4(-quaternion[0], -quaternion[1], -quaternion[2], quaternion[3]);
}

// Turns a rotation into a quaternion
glm::vec4 Realtime::rotation_to_quaternion(glm::vec3 axis, float theta) {
    // Use formula for encoding
    glm::vec3 imag = (float) sin(theta / 2.0f) * axis;
    float real = (float) cos(theta / 2.0f);

    // Create quaternion
    return glm::vec4(imag, real);
}

// Handles rotation of camera
void Realtime::mouseMoveEvent(QMouseEvent *event) {
    // Why thank you for encoding this all in an if statement only if the mouse is pressed
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        // Similar idea to translation: accumulate all rotations, then apply to view matrices
        glm::vec4 rotation(0.0f, 0.0f, 0.0f, 1.0f);

        // Rotation over X axis is relatively easy (since one axis is 0, it's always in world space)
        if (deltaX != 0) {
            // Determine amount of rotation
            float radians = (float) deltaX * radian_conversion;

            // Axis is 0, 0, 1 (since Z and Y axis are flipped in OpenGL)
            glm::vec4 quat_to_rotate = rotation_to_quaternion(glm::vec3(0.0f, 1.0f, 0.0f), -radians);
            rotation = glm::normalize(quaternion_multiply(rotation, quat_to_rotate));
        }

        // Rotation over Y axis it not as easy (we do however, have access to the left and right axis)
        if (deltaY != 0) {
            // Determine amount of rotation
            float radians = (float) deltaY * radian_conversion;

            // Axis is 0, 1, 0
            glm::vec4 quat_to_rotate = rotation_to_quaternion(glm::vec3(m_camera.get_left()), radians);
            rotation = glm::normalize(quaternion_multiply(rotation, quat_to_rotate));
        }

        // Apply rotation updates to camera
        if (deltaX != 0 || deltaY != 0) {
            // Compute newly rotated look and up vectors that define camera rotation matrix
            glm::vec4 new_look = glm::vec4(quaternion_rotate(glm::vec3(m_camera.get_camera_look()), rotation), 0.0f);
            glm::vec4 new_up = glm::vec4(quaternion_rotate(glm::vec3(m_camera.get_camera_up()), rotation), 0.0f);

            // Apply these changes to the view matrix
            m_camera.update_rotation_matrix(new_look, new_up);
            m_camera.update_view_matrix();
        }

        update(); // asks for a PaintGL() call to occur
    }
}

// Handles translation of camera
void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around (well, yes)

    // Strategy: accumulate all positional changes from each key
    glm::vec4 delta_pos(0.0f);

    // If W pressed, move along look
    if (m_keyMap[Qt::Key_W]) {
        delta_pos += m_camera.get_camera_look();
    }

    // If S pressed, move opposite look
    if (m_keyMap[Qt::Key_S]) {
        delta_pos -= m_camera.get_camera_look();
    }

    // If A pressed, move left
    if (m_keyMap[Qt::Key_A]) {
        delta_pos += m_camera.get_left();
    }

    // If D pressed, move right
    if (m_keyMap[Qt::Key_D]) {
        delta_pos += m_camera.get_right();
    }

    // If SPACE pressed, move in world space
    if (m_keyMap[Qt::Key_Space]) {
        delta_pos += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    }

    // If CTRL pressed, move in world space
    if (m_keyMap[Qt::Key_Control]) {
        delta_pos += glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    }

    // Check if any keys were actually pressed (any change happened)
    if (glm::length(delta_pos) > 0.1) {
        // Scale the amount of movement accordingly
        delta_pos = deltaTime * 5.0f * glm::normalize(delta_pos);

        // Update the position and generate a new view matrix
        m_camera.update_translation_matrix(m_camera.get_camera_pos() + delta_pos);
        m_camera.update_view_matrix();
    }

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Adjust FBO we render to
    GLuint old_fbo = default_fbo;
    default_fbo = fbo;

    // resize the openGL viewport and propagate new default FBO
    resizeGL(fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    default_fbo = old_fbo;
    resizeGL(size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

void printMatrix(const glm::mat4& matrix) {
    std::cout << glm::to_string(matrix) << std::endl;
}

// I take it this generates NUMBER amount of random model matrices? Nice
std::vector<glm::mat4> Realtime::generateAsteroidTransformations(const unsigned int number) {
    const float radius = 100.0f;
    const float radiusDeviation = 25.0f;
    std::vector<glm::mat4> instanceMatrix;

    auto randf = []() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    };

    for (unsigned int i = 0; i < number; i++) {
        // Generate a random seed
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distribution(1, 10000);
        int randomSeed = distribution(gen);

        // Initialize FastNoise with the random seed
        FastNoise perlinNoise;
        perlinNoise.SetSeed(randomSeed);

        // Generate x and y using Perlin noise
        float x = randf();
        float y = ((rand() % 2) * 2 - 1) * sqrt(1.0f - x * x);
        float finalRadius = radius + randf() * radiusDeviation;

        // Sample Perlin noise as a height map
        float heightValue = perlinNoise.GetNoise(x * finalRadius, y * finalRadius, 0.0f);

        // Use height value to displace asteroids vertically
        float verticalOffset = heightValue * finalRadius / 2; // recheck this

        // Choose translation axis based on a random distribution
        glm::vec3 translationAxis = (randf() > 0.5f) ?
                                        glm::vec3(y, 0.0f, x) :
                                        glm::vec3(x, 0.0f, y);

        // Holds transformations before multiplying them
        glm::vec3 tempTranslation = translationAxis * finalRadius;
        glm::quat tempRotation = glm::quat(1.0f, randf(), randf(), randf());
        glm::vec3 tempScale = 0.1f * glm::vec3(randf(), randf(), randf());

        // Initialize matrices
        glm::mat4 trans = glm::translate(glm::mat4(1.0f),
                                         tempTranslation + glm::vec3(0.0f, verticalOffset, 0.0f));
        glm::mat4 rot = glm::mat4_cast(tempRotation);
        glm::mat4 sca = glm::scale(glm::mat4(1.0f), tempScale);

        // Push matrix transformation
        instanceMatrix.push_back(trans * rot * sca);
    }

//    // Print out the matrices
//    for (const auto& matrix : instanceMatrix) {
//        std::cout << glm::to_string(matrix) << std::endl;
//    }

    return instanceMatrix;
}
