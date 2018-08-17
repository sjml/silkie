#version 330 core

in vec3 position_world;
in vec3 normal_camera;
in vec3 eye_dir_camera;
in vec3 light_dir_camera;

out vec4 color;

uniform mat4 mv;
uniform vec3 light_position_world;

void main() {
    vec4 light_color = vec4(1,1,1,1);
    float light_power = 50.0f;

    vec4 diffuse = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 ambient = vec4(0.1,0.1,0.1,1.0) * diffuse;
    vec4 specular = vec4(0.3,0.3,0.3,1.0);

    float distance = length(light_position_world - position_world);

    vec3 n = normalize(normal_camera);
    vec3 l = normalize(light_dir_camera);
    float cos_theta = clamp(dot(n, l), 0.0, 1.0);

    vec3 eye_vec = normalize(eye_dir_camera);
    vec3 ref = reflect(-l, n);
    float cos_alpha = clamp(dot(eye_vec, ref), 0.0, 1.0);

    color =
        ambient
        + diffuse * light_color * light_power * cos_theta / (distance * distance)
        + specular * light_color * light_power * pow(cos_alpha, 5) / (distance * distance)
    ;
}
