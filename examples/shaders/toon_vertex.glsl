#version 330 core

precision highp float;

uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;
uniform vec3 light_position_world;

layout(location = 0) in vec3 vertex_position_model;
layout(location = 1) in vec3 vertex_normal_model;

out vec3 normal_camera;
out vec3 light_dir_camera;

void main() {
    gl_Position = mvp * vec4(vertex_position_model, 1.0);

    vec3 vertex_position_camera = (view * model * vec4(vertex_position_model, 1.0)).xyz;
    vec3 eye_dir_camera = vec3(0.0, 0.0, 0.0) - vertex_position_camera;
    vec3 light_position_camera = (view * vec4(light_position_world, 1.0)).xyz;

    light_dir_camera = light_position_camera + eye_dir_camera;
    normal_camera = (view * model * vec4(vertex_normal_model, 0.0)).xyz;
}
