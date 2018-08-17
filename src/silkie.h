#ifndef SILKIE_H
#define SILKIE_H

#if SILKIE_OFFSCREEN
    #include <GL/gl.h>
    #include <GL/glext.h>
#else
    #ifdef __APPLE__
        #ifdef SILKIE_GL_THREE_PLUS
            #include <OpenGL/gl3.h>
        #else
            #include <OpenGL/gl.h>
        #endif
    #else
        #include <GL/gl.h>
    #endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

void* silkie_get_context(int width, int height, int oglMajorVersion=1, int oglMinorVersion=5);

void silkie_register_update(int (*uf)(void*, double));
void silkie_register_render(void (*rf)(void*));

int silkie_run(void* ctx, const char* output_location, int framerate);

#ifdef __cplusplus
}
#endif

#endif
