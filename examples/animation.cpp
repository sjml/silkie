// Takes the triangle from the basic example and spins it once each around the Y and X axes.

// Some metrics are at the bottom of the file. :)

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#include <silkie.h>

GLfloat tri_rotation = 0.0f;

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
    tri_rotation += dt * 90.0f;
    if (tri_rotation <= 360.0f) {
        return 1;
    }
    else {
        return 0;
    }
}

void render(void* ctx) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f,0.0f,-1.0f);
    if (tri_rotation <= 180.0f) {
        glRotatef(tri_rotation, 0.0f, 1.0f, 0.0f);
    }
    else {
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(tri_rotation - 180.0f, 1.0f, 0.0f, 0.0f);
    }

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

    void* ctx = silkie_get_context(width, height);

    if (ctx == NULL) {
        fprintf(stderr, "ERROR: No context created.\n");
        return 1;
    }

    setup();

    silkie_register_update(update);
    silkie_register_render(render);

    int run_val = silkie_run(ctx, output, fps);

    return run_val;
}


// Timings:
    // Program represents 4.0 seconds of animation. 240 frames at 60fps.
    // This was done without closing file handles until the program exited.
    // (A bug at the time; haven't rerun these tests since.)
    // 13" MacBook Pro 3.1 GHz Intel Core i5:
        // 640x480: renders in 4.45s (average)
        // 800x600: renders in 6.53s (average)
        // 1920x1080: renders in 25.35 (average)
        // 3840x2160: renders 102.67s (average)

    // Uses PNG encoding: uncompressed files are substantially slower;
    //    writing to disk is the bottleneck, not the compression
    //
    // Different PNG libraries were experimented with. By this time the file
    // handle bug had been fixed, so base times are higher.
    // Running this same scene at 1928x1080:
    //
        // libpng, default settings: 68.82s (average)
        // miniz, no compression: 27.17s (average)
        // lodepng, default settings: 28.12s (average)
        // stb_image_write, no compresison: 37.89s (average)
        // stb_image_write, highest compression: 46.02s (average)
        // miniz, highest compression: 61.81s (average)
    //
    // File sizes (of basic scene @ 640x480):
        // libpng: 13688 bytes
        // lodepng: 13831 bytes
        // stb_image_write, highest compression: 26920 bytes
        // stb_image_write, lowest compression: 27539 bytes
        // miniz, highest compression: 183389 bytes
        // miniz, no compression: 488311 bytes
    //
    // LodePNG is the fastest and smallest implementation, but my trust in the quality
    //  of Sean Barrett's work leads me to prefer stb_image_write and its small penalties.
    //  Note these scenes do not really stress-test the PNG encoder, and were not performed
    //  in laboratory settings. All rough metrics.
    //
    // This is a trivial scene; obviously complicated ones will take longer.
    //   These metrics just represent the base case.
