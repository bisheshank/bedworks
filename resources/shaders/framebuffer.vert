#version 330 core

// Takes in a position on screen (for our fullscreen cover) and the input texture
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_in;

// Outputs texture coordinates (done using linear interpolation)
out vec2 tex_coord;

void main() {
    // Assign texture coordinates
    tex_coord = tex_in;

    // Do not change the position to render anything on (this is merely postprocesing)
    gl_Position = vec4(position, 1.0);
}
