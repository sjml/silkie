#version 330 core

precision highp float;

in vec3 normal_camera;
in vec3 light_dir_camera;

out vec4 color;

void main() {
    float intensity = dot(light_dir_camera, normalize(normal_camera));

    if (intensity > 0.95) {
        color = vec4(0.5, 1.0, 0.5, 1.0);
    }
    else if (intensity > 0.5) {
        color = vec4(0.3, 0.6, 0.3, 1.0);
    }
    else if (intensity > 0.25) {
        color = vec4(0.2, 0.4, 0.2, 1.0);
    }
    else {
        color = vec4(0.1, 0.2, 0.1, 1.0);
    }
}
