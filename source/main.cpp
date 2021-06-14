#include <chrono>
#include <cstdio>
#include <cstring>
#include "application.h"
#include "example_base.h"

using namespace rs;
using namespace std::chrono;

static void updateTitle(float ms) {
    static char s_buf[256];
    snprintf(s_buf, sizeof(s_buf), "%s (FPS: %d)", g_config.title, int(1000.f / ms));
    app::setWindowTitle(s_buf);
}

int main(int, char**) {
    g_pExample->initialize();
    app::initialize();

    uint64_t tick = 0;
    system_clock::time_point lastTimePoint = system_clock::now();
    system_clock::time_point currTimePoint = system_clock::now();
    uint64_t nano = 0;
    for (;;) {
        app::pollEvents();
        if (app::shouldQuit()) {
            break;
        }

        // render
        currTimePoint = system_clock::now();
        nano = duration_cast<nanoseconds>(currTimePoint - lastTimePoint).count();
        g_pExample->update(nano * 1.0e-9);

        app::drawPixels(g_config.width,
                        g_config.height,
                        g_pExample->getTexture().getData());

        lastTimePoint = currTimePoint;
        updateTitle(float(nano * 1.0e-6));
        // printf("frame %llu took %lfms\n", ++tick, nano * 1.0e-6);
    }

    app::finalize();

    return 0;
}
