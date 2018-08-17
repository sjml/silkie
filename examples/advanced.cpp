// Loads an obj model and renders it with shaders.
//   Necessitates some more modern OpenGL usage.

// C headers
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

// C++ headers
#include <vector>

// Project headers
#define SILKIE_GL_THREE_PLUS // Need this on the Mac if you're using GL3 or higher
#include <silkie.h>

// single-file includes
#define GB_MATH_IMPLEMENTATION
#include "gb_math.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


std::vector<gbVec3> vertices;
std::vector<gbVec3> normals;
float aspect;
GLuint vao;
GLuint shader;
gbMat4 mvp; GLuint mvp_location;
gbMat4 model_matrix; GLuint model_location;
gbMat4 view_matrix; GLuint view_location;
gbMat4 projection_matrix; GLuint projection_location;
GLuint light_location;
GLuint vertex_buffer;
GLuint normal_buffer;


// quick and dirty; won't work on very large files
char* string_from_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        return 0;
    }
    size_t read_count = fread(buffer, 1, length, f);
    fclose(f);
    if (read_count == 0) {
        return NULL;
    }
    buffer[length] = '\0';
    return buffer;
}

GLuint load_shader(const char* vertex_file, const char* fragment_file) {
    const char* vertex_shader = string_from_file(vertex_file);
    const char* fragment_shader = string_from_file(fragment_file);

    if (vertex_shader == 0 || fragment_shader == 0) {
        return 0;
    }

    GLint success;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("ERROR: couldn't compile %s:\n", vertex_file);
        char shader_log[4096];
        GLsizei log_length;
        glGetShaderInfoLog(vs, 4096, &log_length, shader_log);
        fprintf(stderr, "\t%s\n", shader_log);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("ERROR: couldn't compile %s:\n", fragment_file);
        char shader_log[4096];
        GLsizei log_length;
        glGetShaderInfoLog(fs, 4096, &log_length, shader_log);
        fprintf(stderr, "\t%s\n", shader_log);
    }

    GLuint shader_index = glCreateProgram();
    glAttachShader(shader_index, fs);
    glAttachShader(shader_index, vs);
    glLinkProgram(shader_index);
    glGetProgramiv(shader_index, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        fprintf(stderr, "ERROR: couldn't link %s and %s:\n", vertex_file, fragment_file);
        char shader_log[4096];
        GLsizei log_length;
        glGetProgramInfoLog(shader, 4096, &log_length, shader_log);
        fprintf(stderr, "\t%s\n", shader_log);
    }

    free((void*)vertex_shader);
    free((void*)fragment_shader);

    return shader_index;
}

bool loadOBJ(const char * path, std::vector<gbVec3> & out_vertices, std::vector<gbVec3> & out_normals) {
    std::string error_check;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &error_check, path);

    if (!error_check.empty()) {
        fprintf(stderr, "TinyOBJ: %s", error_check.c_str());
    }
    if (!success) {
        return false;
    }

    for (int i=0; i < shapes[0].mesh.indices.size(); i++) {
        gbVec3 v;
        v.x = attrib.vertices[3 * shapes[0].mesh.indices[i].vertex_index + 0];
        v.y = attrib.vertices[3 * shapes[0].mesh.indices[i].vertex_index + 1];
        v.z = attrib.vertices[3 * shapes[0].mesh.indices[i].vertex_index + 2];
        out_vertices.push_back(v);

        gbVec3 normal;
        normal.x = attrib.normals[3 * shapes[0].mesh.indices[i].normal_index + 0];
        normal.y = attrib.normals[3 * shapes[0].mesh.indices[i].normal_index + 1];
        normal.z = attrib.normals[3 * shapes[0].mesh.indices[i].normal_index + 2];
        gb_vec3_norm(&normal, normal);
        out_normals.push_back(normal);
    }
    return true;
}

int setup(int w, int h) {
    char* file_path = strdup(__FILE__);
    std::string sample_path = dirname(file_path);

    aspect = (float) w / (float) h;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    std::string vpath = sample_path + "/shaders/toon_vertex.glsl";
    std::string fpath = sample_path + "/shaders/toon_fragment.glsl";
    shader = load_shader(vpath.c_str(), fpath.c_str());

    glUseProgram(shader);
    view_location = glGetUniformLocation(shader, "view");
    model_location = glGetUniformLocation(shader, "model");
    mvp_location = glGetUniformLocation(shader, "mvp");
    light_location = glGetUniformLocation(shader, "light_position_world");


    std::string obj_path = sample_path + "/data/teapot.obj";

    loadOBJ(obj_path.c_str(), vertices, normals);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(gbVec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(gbVec3), &normals[0], GL_STATIC_DRAW);

    return 0;
}

int update(void* ctx, double dt) {
    static float rotation = 0.0f;
    rotation += dt * 90.0f;

    gbMat4 rotate;
    gb_mat4_rotate(&rotate, (gbVec3) {0.0f, 1.0f, 0.0f}, gb_to_radians(rotation));
    gbMat4 translate;
    gb_mat4_translate(&translate, (gbVec3) {0.0f, -1.4f, 0.0f});

    gb_mat4_identity(&model_matrix);
    gb_mat4_mul(&model_matrix, &rotate, &model_matrix);
    gb_mat4_mul(&model_matrix, &translate, &model_matrix);

    gb_mat4_perspective(&projection_matrix,
        45.0,
        aspect,
        0.001f,
        100.0f
    );
    gb_mat4_look_at(&view_matrix,
        (gbVec3) { 0.0f, 0.0f, 7.0f },
        (gbVec3) { 0.0f, 0.0f, 1.0f },
        (gbVec3) { 0.0f, 1.0f, 0.0f }
    );
    gb_mat4_mul(&mvp, &view_matrix, &model_matrix);
    gb_mat4_mul(&mvp, &projection_matrix, &mvp);

    if (rotation <= 360.0f) {
        return 1;
    }
    else {
        return 0;
    }
}

void render(void* ctx) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp.e[0]);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, &model_matrix.e[0]);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, &view_matrix.e[0]);

    gbVec3 light_pos = (gbVec3) {2.0f, 1.0f, 2.5f};
    glUniform3f(light_location, light_pos.x, light_pos.y, light_pos.z);

    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

int main(int argc, char *argv[])
{
    int opt;
    int width = 640;
    int height = 480;
    int fps = 60;
    const char* output = "out";
    errno = 0;

    while ((opt = getopt(argc, argv, "w:h:f:o:")) != -1) {
        switch (opt) {
            case 'w': {
                intmax_t w = strtoimax(optarg, NULL, 10);
                if (errno) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    return 1;
                }
                if (w < 0) {
                    fprintf(stderr, "Width must be positive integer.\n");
                    return 1;
                }
                width = w;
                break;
            }
            case 'h': {
                intmax_t h = strtoimax(optarg, NULL, 10);
                if (errno) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    return 1;
                }
                if (h < 0) {
                    fprintf(stderr, "Height must be positive integer.\n");
                    return 1;
                }
                height = h;
                break;
            }
            case 'f': {
                intmax_t f = strtoimax(optarg, NULL, 10);
                if (errno) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    return 1;
                }
                if (f < 0) {
                    fprintf(stderr, "FPS must be positive integer.\n");
                    return 1;
                }
                fps = f;
                break;
            }
            case 'o': {
                // will get checked before run
                output = optarg;
                break;
            }
        }
    }

    void* ctx = silkie_get_context(width, height, 3, 2);

    if (ctx == NULL) {
        fprintf(stderr, "ERROR: No context created.\n");
        return 1;
    }

    int setup_val = setup(width, height);
    if (setup_val) {
        return setup_val;
    }

    silkie_register_update(update);
    silkie_register_render(render);

    int run_val = silkie_run(ctx, output, fps);

    return run_val;
}
