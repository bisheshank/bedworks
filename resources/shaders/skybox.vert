#version 330 core
layout (location = 0) in vec3 iPos;

out vec3 texture_coords;

uniform mat4 proj_matrix;
uniform mat4 view_matrix;

void main()
{
    vec4 pos = proj_matrix * view_matrix * vec4(iPos, 1.0f);
    // Having z equal w will always result in a depth of 1.0f
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
    // TRUST
    texture_coords = vec3(iPos.x, iPos.y, -iPos.z);
}
