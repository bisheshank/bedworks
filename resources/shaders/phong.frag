#version 330 core
// Number of lights to support
const int num_lights = 8;

// Inputs: Position and normal for model's point in world space
in vec3 world_position;
in vec3 world_normal;

// Outputs: The color for this fragment (pixel-esque thingy)
out vec4 frag_color;

// Struct definition of lights as uniform
struct Light {
    vec4 color;
    vec3 pos;
    vec3 attenuation_func;
    vec4 dir;
    float penumbra;
    float angle;
    // Type indicates what kind of light this is
    // 0 -> Directional
    // 1 -> Point
    // 2 -> Spot
    // -1 -> Light doesn't exist
    int type;
};

// Uniform for lights
uniform Light lights[num_lights];

// Uniform for lighting computation (global coeffs)
uniform float ka;
uniform float kd;
uniform float ks;

// Uniform material properties of primitive
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float shininess;

// Uniform positions required
uniform vec3 camera_pos;

// Helper to compute Phong lighting for directional lights
vec3 compute_directional_light(Light light, vec3 normal) {
    // Some helpful precomputations
    vec3 light_color = vec3(light.color);
    vec3 light_direction = normalize(vec3(light.dir));
    vec3 accumulated_color = vec3(0.0);
    vec3 reflected_light = normalize(reflect(light_direction, normal));
    vec3 normal_camera_pos = normalize(vec3(camera_pos) - world_position);

    // Compute diffuse and specular coeffs
    float diffuse_coeff = clamp(dot(-light_direction, normal), 0.0, 1.0);
    diffuse_coeff *= kd;
    float specular_coeff = clamp(dot(normal_camera_pos, reflected_light), 0.0, 1.0);

    // Don't invoke undefined behavior
    if (shininess >= 0.01 || specular_coeff >= 0.01) {
        specular_coeff = pow(specular_coeff, shininess) * ks;
    }

    // Add the colors
    accumulated_color += diffuse_coeff * vec3(diffuse);
    accumulated_color += specular_coeff * vec3(specular);

    // The LIGHT HAS IMPACT
    accumulated_color *= light_color;

    return accumulated_color;
}

// Helper to compute Phong lighting for point lights
vec3 compute_point_light(Light light, vec3 normal) {
    // Helpful precomputation to apply Phong lighting equation (as in directional lights)
    vec3 light_color = vec3(light.color);
    vec3 light_direction = normalize(world_position - light.pos);
    vec3 accumulated_color = vec3(0.0);
    vec3 reflected_light = normalize(reflect(light_direction, normal));
    vec3 normal_camera_pos = normalize(vec3(camera_pos) - world_position);

    // Compute attenuation coefficient
    float distance = length(world_position - light.pos);
    float attenuation = 1.0 / (light.attenuation_func[0] + (distance * light.attenuation_func[1]) + (distance * distance * light.attenuation_func[2]));
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Compute diffuse and specular coeffs
    float diffuse_coeff = clamp(dot(-light_direction, normal), 0.0, 1.0);
    diffuse_coeff *= kd;
    float specular_coeff = clamp(dot(normal_camera_pos, reflected_light), 0.0, 1.0);

    // Don't invoke undefined behavior
    if (shininess >= 0.01 || specular_coeff >= 0.01) {
        specular_coeff = pow(specular_coeff, shininess) * ks;
    }

    // Add the colors
    accumulated_color += diffuse_coeff * vec3(diffuse);
    accumulated_color += specular_coeff * vec3(specular);

    // The LIGHT HAS IMPACT
    accumulated_color *= light_color;
    accumulated_color *= attenuation;

    return accumulated_color;
}

// Helper to compute Phong lighting for spot lights
vec3 compute_spot_light(Light light, vec3 normal) {
    // Helpful precomputation to apply Phong lighting equation (as in spot lights)
    vec3 light_color = vec3(light.color);
    vec3 light_to_position = normalize(world_position - light.pos);
    vec3 light_dir = vec3(light.dir);
    vec3 accumulated_color = vec3(0.0);
    vec3 reflected_light = normalize(reflect(light_to_position, normal));
    vec3 normal_camera_pos = normalize(vec3(camera_pos) - world_position);

    // Determine angle between illuminated point and the light's position
    float theta = acos(dot(normalize(light_dir), light_to_position));

    // If light falls outside the spotlight's illumination, do not compute lighting
    if (theta > light.angle) {
        return vec3(0.0);
    }

    // Determine how much falloff we need
    float falloff = 1.0;

    // Only apply falloff if located in outer cone of light
    float inner_cone = light.angle - light.penumbra;
    if (inner_cone < theta) {
        float diff = (theta - inner_cone) / light.penumbra;
        diff = (-2 * pow(diff, 3)) + (3 * pow(diff, 2));
        falloff = 1.0 - diff;
    }

    // Compute attenuation coefficient
    float distance = length(world_position - light.pos);
    float attenuation = 1.0 / (light.attenuation_func[0] + (distance * light.attenuation_func[1]) + (distance * distance * light.attenuation_func[2]));
    // Incorporate falloff
    attenuation = attenuation * falloff;
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Compute diffuse and specular coeffs
    float diffuse_coeff = clamp(dot(-light_to_position, normal), 0.0, 1.0);
    diffuse_coeff *= kd;
    float specular_coeff = clamp(dot(normal_camera_pos, reflected_light), 0.0, 1.0);

    // Don't invoke undefined behavior
    if (shininess >= 0.01 || specular_coeff >= 0.01) {
        specular_coeff = pow(specular_coeff, shininess) * ks;
    }

    // Add the colors
    accumulated_color += diffuse_coeff * vec3(diffuse);
    accumulated_color += specular_coeff * vec3(specular);

    // The LIGHT HAS IMPACT
    accumulated_color *= light_color;
    accumulated_color *= attenuation;

    return accumulated_color;
}

void main() {
    // PLACEHOLODER CODE: Returns white
    // frag_color = vec4(1.0);

    // Initialize worldspace normal
    vec3 new_world_normal = normalize(world_normal);

    // Start with the ambient color
    vec3 accumulated_color = vec3(0.0);
    accumulated_color += ka * vec3(ambient);

    // Accumulate color for all lights
    for (int i = 0; i < num_lights; i++) {
        // Switch case based on light type
        // Directional
        if (lights[i].type == 0) {
            accumulated_color += compute_directional_light(lights[i], new_world_normal);
        }
        // Point
        else if (lights[i].type == 1) {
            accumulated_color += compute_point_light(lights[i], new_world_normal);
        }
        // Spot
        else if (lights[i].type == 2) {
            accumulated_color += compute_spot_light(lights[i], new_world_normal);
        }
    }

    // Convert back to vec4 and return
    frag_color = vec4(accumulated_color, 1.0);
}
