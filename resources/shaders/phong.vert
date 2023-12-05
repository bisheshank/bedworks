#version 330 core

// Take in position and normal from object space
layout(location = 0) in vec3 object_pos;
layout(location = 1) in vec3 object_normal;

// Output position and normal in world space (this just converts things to world space)
out vec3 world_position;
out vec3 world_normal;

// Uniforms for computing transformations related to model (position and normal)
uniform mat4 model_matrix;
uniform mat3 inverse_model_normal_matrix;

// Uniforms for computing the gl_Position var (determines where object appears on screen)
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

void main() {
    // Convert object to homogeneous coordinates so we can apply transform
    vec4 homo_object_pos = vec4(object_pos, 1.0);
    world_position = vec3(model_matrix * homo_object_pos);

    // Compute the normal using the inverse_model_normal_matrix
    world_normal = normalize(inverse_model_normal_matrix * object_normal);

    // Compute gl_Position
    gl_Position = (proj_matrix * view_matrix * model_matrix) * homo_object_pos;
}
