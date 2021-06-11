#include "RenderTarget.h"

namespace rs {

void RenderTarget::resize(int width, int height) {
    m_colorBuffer.resize(width, height);
    m_depthBuffer.resize(width, height);
}

void RenderTarget::create(const CreateInfo& info) {
    resize(info.width, info.height);
    m_useColor = info.useColor;
    m_useDepth = info.useDepth;
}

}  // namespace rs
