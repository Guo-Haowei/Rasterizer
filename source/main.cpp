#include <chrono>
#include <cstdio>
#include <cstring>
#include "application.h"
#include "example_base.h"

using namespace rs;

using namespace std::chrono;

static void updateTitle(float ms) {
    static char s_buf[256];
    snprintf(s_buf, sizeof(s_buf), "%s (FPS: %d)", g_pExample->getTitle().c_str(), int(1000.f / ms));
    setWindowTitle(s_buf);
}

int main(int, char**) {
    initialize();

    uint64_t tick = 0;
    system_clock::time_point lastTimePoint = system_clock::now();
    system_clock::time_point currTimePoint = system_clock::now();
    uint64_t nano = 0;
    for (;;) {
        pollEvents();
        if (shouldQuit()) {
            break;
        }

        // render
        currTimePoint = system_clock::now();
        nano = duration_cast<nanoseconds>(currTimePoint - lastTimePoint).count();
        g_pExample->update(nano * 1.0e-9);

        drawPixels(g_pExample->getWidth(),
                   g_pExample->getHeight(),
                   g_pExample->getTexture().getData());

        lastTimePoint = currTimePoint;
        updateTitle(float(nano * 1.0e-6));
        // printf("frame %llu took %lfms\n", ++tick, nano * 1.0e-6);
    }

    finalize();

    return 0;
}
