// draws a multi-colored triangle, the classic fixed-function OpenGL demo
//   exits after 1.5 seconds, or the user manually closing the window

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#include <silkie.h>

void setup() {
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-0.5, 0.5, -0.5, 0.5, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int update(void* ctx, double dt) {
    // just render a single frame
    static bool done = false;
    if (!done) {
        done = true;
        return 1;
    }
    return 0;
}

void render(void* ctx) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f,0.0f,-1.0f);

    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(-0.5f, -0.5f);
        glColor3f(0.0f, 1.0f, 0.0f); glVertex2f( 0.5f, -0.5f);
        glColor3f(0.0f, 0.0f, 1.0f); glVertex2f( 0.0f,  0.5f);
    glEnd();
}

int main(int argc, char *argv[])
{
    int opt;
    int width = 640;
    int height = 480;
    const char* output = "out";
    errno = 0;
    while ((opt = getopt(argc, argv, "w:h:o:")) != -1) {
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
            case 'o': {
                // will get checked before run
                output = optarg;
                break;
            }
        }
    }

    void* ctx = silkie_get_context(width, height);

    if (ctx == NULL) {
        fprintf(stderr, "ERROR: No context created.\n");
        return 1;
    }

    setup();

    silkie_register_update(update);
    silkie_register_render(render);

    int run_val = silkie_run(ctx, output, 60);

    return run_val;
}
