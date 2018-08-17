#version 330 core

in vec3 position_world;
in vec3 normal_camera;
in vec3 eye_dir_camera;
in vec3 light_dir_camera;

out vec3 color;

uniform mat4 mv;
uniform vec3 light_position_world;

void main() {
    vec3 light_color = vec3(1,1,1);
    float light_power = 50.0f;

    vec3 diffuse = vec3(1.0, 0.0, 0.0);
    vec3 ambient = vec3(0.1,0.1,0.1) * diffuse;
    vec3 specular = vec3(0.3,0.3,0.3);

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
