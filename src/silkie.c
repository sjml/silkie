#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>

#if SILKIE_OFFSCREEN
    #include <GL/osmesa.h>

    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "stb_image_write.h"
#else
    #include <GLFW/glfw3.h>
#endif

#if SILKIE_OFFSCREEN
    typedef struct {
        OSMesaContext ctx;
        GLubyte* buffer;
        int width;
        int height;
    } _silkie_context;
#endif

// Just drawing the line someplace. I can't imagine anyone needing to render
//   any larger images than this, but if you do, here's an easy number to change.
#define SILKIE_MAX_IMAGE_SIZE 65536

int  (*_silkie_update_function)(void*, double) = NULL;
void (*_silkie_render_function)(void*) = NULL;

void* silkie_get_context(int width, int height, int oglMajorVersion, int oglMinorVersion) {
    if (
        (width < 0) || (height < 0) ||
        (width > SILKIE_MAX_IMAGE_SIZE) || (height > SILKIE_MAX_IMAGE_SIZE)
        ) {
        return NULL;
    }

    #if SILKIE_OFFSCREEN
        _silkie_context* sc = calloc(1, sizeof(_silkie_context));
        (*sc).width = width;
        (*sc).height = height;

        int prof_type = OSMESA_COMPAT_PROFILE;
        if (
            ((oglMajorVersion == 3) && (oglMinorVersion >= 2))
            || oglMajorVersion > 3
           ) {
            prof_type = OSMESA_CORE_PROFILE;
        }

        const int attribs[] = {
            OSMESA_FORMAT, OSMESA_RGBA, // <sigh> OSMesa doesn't do RGB ubyte
            OSMESA_DEPTH_BITS, 32,
            OSMESA_STENCIL_BITS, 0,
            OSMESA_ACCUM_BITS, 0,
            OSMESA_PROFILE, prof_type,
            OSMESA_CONTEXT_MAJOR_VERSION, oglMajorVersion,
            OSMESA_CONTEXT_MINOR_VERSION, oglMinorVersion,
            0
        };
        (*sc).ctx = OSMesaCreateContextAttribs(attribs, NULL);
        if (!(*sc).ctx) {
            free(sc);
            return NULL;
        }

        (*sc).buffer = (GLubyte *) malloc( (*sc).width * (*sc).height * 4 * sizeof(GLubyte));
        if (!(*sc).buffer) {
            OSMesaDestroyContext((*sc).ctx);
            free(sc);
            return NULL;
        }

        if (!OSMesaMakeCurrent((*sc).ctx, (*sc).buffer, GL_UNSIGNED_BYTE, (*sc).width, (*sc).height)) {
            OSMesaDestroyContext((*sc).ctx);
            free((*sc).buffer);
            free(sc);
            return NULL;
        }

        glViewport(0, 0, (*sc).width, (*sc).height);

        return (void*)sc;
    #else
        GLFWwindow* window;

        if (!glfwInit()) {
            return NULL;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, oglMajorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, oglMinorVersion);
        if (
            ((oglMajorVersion == 3) && (oglMinorVersion >= 2))
            || oglMajorVersion > 3
           ) {
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        else {
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        }

        window = glfwCreateWindow(width, height, "Silkie Window", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return NULL;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        return (void*)window;
    #endif
}


void silkie_register_update(int (*uf)(void*, double)) {
    _silkie_update_function = uf;
}

void silkie_register_render(void (*rf)(void*)) {
    _silkie_render_function = rf;
}

double _silkie_get_time() {
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

int silkie_run(void* ctx, const char* output_location, int framerate) {
    if (_silkie_update_function == NULL) {
        fprintf(stderr, "ERROR: No update function registered; abandoning run.\n");
        return 1;
    }
    if (_silkie_render_function == NULL) {
        fprintf(stderr, "ERROR: No render function registered; abandoning run.\n");
        return 1;
    }
    if (ctx == NULL) {
        fprintf(stderr, "ERROR: Null native context; abandoning run.\n");
        return 1;
    }

    #if SILKIE_OFFSCREEN
        // all these checks and the file separator stuff will have to be modified if
        //   this ever needs to run on Windows
        if (output_location == NULL || strlen(output_location) == 0 || output_location[0] == '\0') {
            fprintf(stderr, "ERROR: No output location given.\n");
            return 1;
        }
        struct stat st = {0};
        if (stat(output_location, &st) != 0) {
            fprintf(stderr, "ERROR: Output location does not exist: '%s'.\n", output_location);
            return 1;
        }
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "ERROR: Output location is not a directory: '%s'.\n", output_location);
            return 1;
        }
        if (!(st.st_mode & S_IWUSR)) {
            fprintf(stderr, "ERROR: Output location not writable: '%s'.\n", output_location);
            return 1;
        }

        char* out = strdup(output_location);
        int len = strlen(out);
        char last = out[len-1];
        if (last != '/') {
            strcat(out, "/");
        }

        const double framestep = 1.0 / (double)framerate;

        _silkie_context* sctx = (_silkie_context*)ctx;
    #endif

    int cont = 1;
    double lastTime = 0.0f;
    double currentTime = 0.0f;
    currentTime = lastTime = _silkie_get_time();

    // more than sufficient, perhaps overkill?
    unsigned long long frameCounter = 0;
    while (cont) {
        double dt = 0.0;

        #if SILKIE_OFFSCREEN
            dt = framestep;
        #else
            currentTime = _silkie_get_time();
            dt = currentTime - lastTime;
            lastTime = currentTime;
        #endif

        #if SILKIE_OFFSCREEN
            OSMesaMakeCurrent((*sctx).ctx, (*sctx).buffer, GL_UNSIGNED_BYTE, (*sctx).width, (*sctx).height);
        #else
            glfwMakeContextCurrent((GLFWwindow*)ctx);
        #endif

        cont = _silkie_update_function(ctx, dt);
        if (cont) {
            _silkie_render_function(ctx);

            #if SILKIE_OFFSCREEN
                glFinish(); // make sure all the buffers are flushed

                char filename[PATH_MAX];
                snprintf(filename, PATH_MAX, "%s%llu.png", out, frameCounter++);

                stbi_flip_vertically_on_write(1);
                stbi_write_png_compression_level = 8; //default = 8
                stbi_write_png(filename, (*sctx).width, (*sctx).height, 4, (*sctx).buffer, (*sctx).width * 4);
            #else
                glfwSwapBuffers((GLFWwindow*)ctx);
                glfwPollEvents();
                if (glfwWindowShouldClose((GLFWwindow*)ctx)) {
                    break;
                }
            #endif
        }

    }

    #if SILKIE_OFFSCREEN
        OSMesaDestroyContext((*sctx).ctx);
        free((*sctx).buffer);
        free(sctx);
    #else
        glfwDestroyWindow((GLFWwindow*)ctx);
        glfwTerminate();
    #endif

    return 0;
}
