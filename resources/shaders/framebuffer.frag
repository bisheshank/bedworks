#version 330 core

// Takes in a texture coordinate
in vec2 tex_coord;

// Texture (what we've rendered so far) to sample from
uniform sampler2D tex;

// Booleans indicating if we want to perform certain post-processing operations
uniform bool per_pixel;
uniform bool per_kernel;

// Controls how far we move in the UV direction to determine sampling for kernel-based convolution
uniform float u_step;
uniform float v_step;

// Controls radius of convolution
uniform int radius;

out vec4 fragColor;

void main()
{
    // Sample from the texture at that coordinate (no post-processing has occurred)
    fragColor = texture(tex, tex_coord);

    // Apply box blur
    if (per_kernel) {
        vec3 accumulated_color = vec3(0.0);

        // Convolve over surrounding pixels (note that radius is a radius, so we get this funky looking for loop)
        for (int delta_u = -radius; delta_u <= radius; delta_u++) {
            for (int delta_v = -radius; delta_v <= radius; delta_v++) {
                // Compute new texture coordinate to sample from
                float new_u = tex_coord[0] + (delta_u * u_step);
                float new_v = tex_coord[1] + (delta_v * v_step);

                // Accumulate the color at that location (in the texture)
                accumulated_color += vec3(texture(tex, vec2(new_u, new_v)));
            }
        }

        // Scale accumulated color appropriately
        int scale = ((2 * radius) + 1);
        scale = scale * scale;

        // THIS IS THE POWER OF BOX BLURRING
        fragColor.r = accumulated_color.r / scale;
        fragColor.g = accumulated_color.g / scale;
        fragColor.b = accumulated_color.b / scale;
    }

    // Converts pixels to grayscale if using per_pixel operations
    if (per_pixel) {
        // Grayscale computation from before
        float gray = (0.299 * fragColor.r) + (0.587 * fragColor.g) + (0.114 * fragColor.b);

        // Set all color channels to this (that's what it means to grayscale)
        fragColor.r = gray;
        fragColor.g = gray;
        fragColor.b = gray;
    }

}
