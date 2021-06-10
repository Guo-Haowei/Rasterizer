#include "BaseApp.h"
#include <imgui.h>

namespace hguo {

BaseApp::BaseApp(const Config& config)
    : m_width(config.width), m_height(config.height), m_title(config.title) {
    m_renderTarget.create({ m_width, m_height, true, true });
    m_renderer.setRenderTarget(&m_renderTarget);
}

void BaseApp::gui() {
    // ImGui::Begin("Debug");
    // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // ImGui::End();
}

}  // namespace hguo
