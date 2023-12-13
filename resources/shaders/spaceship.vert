#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Normals (not necessarily normalized)
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;


// Outputs the current position for the Fragment Shader
out vec3 crntPos;
// Outputs the normal for the Fragment Shader
out vec3 Normal;
// Outputs the color for the Fragment Shader
out vec3 color;
// Outputs the texture coordinates to the Fragment Shader
out vec2 texCoord;



// Imports the camera matrix
uniform mat4 proj_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_pos;
// Imports the transformation matrices
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;


void main()
{
        // calculates translation of current position relative to camera
        mat4 model_mat = inverse_view_matrix * inverse(translation);
        crntPos = vec3(model_mat * vec4(aPos, 1.0));
        // Assigns the normal from the Vertex Data to "Normal"
        mat3 inverse_model_matrix = inverse(mat3(model_mat));
        Normal = normalize(inverse_model_matrix * aNormal);
        // Assigns the colors from the Vertex Data to "color"
        color = aColor;
        // Assigns the texture coordinates from the Vertex Data to "texCoord"
        texCoord = mat2(0.0, -1.0, 1.0, 0.0) * aTex;

        // Outputs the positions/coordinates of all vertices
        // Fix the object's position in camera space
        gl_Position = proj_matrix * translation * scale * rotation * vec4(aPos, 1.0);
}
