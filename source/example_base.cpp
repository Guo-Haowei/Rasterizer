#include "example_base.h"

ExampleBase::ExampleBase(const Config& config)
    : m_width(config.width), m_height(config.height), m_title(config.title) {
    m_renderTarget.create({ m_width, m_height, true, true });
    m_renderer.setRenderTarget(&m_renderTarget);
}
