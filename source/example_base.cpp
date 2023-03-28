#include "example_base.h"
#include "rasterizer/JobSystem.h"
#include "application.h"
#include <chrono>

using namespace std::chrono;

ExampleBase::ExampleBase(const Config& config)
    : m_width(config.width), m_height(config.height), m_title(config.title) {
}

int ExampleBase::run() {
    jobsystem::initialize();
    rs::initialize();

    m_renderTarget.create({ m_width, m_height, true, true });
    rs::setRenderTarget(&m_renderTarget);
    rs::setSize(m_width, m_height);

    postInit();

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
        update(nano * 1.0e-9);
        app::drawPixels(m_width,
                        m_height,
                        m_renderTarget.getColorBuffer().getData());

        lastTimePoint = currTimePoint;

        char buf[256];
        snprintf(buf, sizeof(buf), "%s (FPS: %d)", g_config.title, int(1000.f / (nano * 1.0e-6)));
        app::setWindowTitle(buf);
        // printf("frame %llu took %lfms\n", ++tick, nano * 1.0e-6);
    }

    app::finalize();
    rs::finalize();
    jobsystem::finalize();

    return 0;
}
