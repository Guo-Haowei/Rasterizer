#pragma once
#include "renderer/Renderer.h"
#include <string>

namespace hguo {

struct Config
{
    int width;
    int height;
    const char* title;
};

class BaseApp
{
public:
    BaseApp(const Config& config);

    virtual void initialize() = 0;

    virtual void update(double deltaTime) = 0;

    virtual void finalize() = 0;

    virtual void gui();

    virtual const Texture& getTexture() const { return m_renderTarget.m_colorBuffer; }

    int getWidth() const { return m_width; }

    int getHeight() const { return m_height; }

    const std::string& getTitle() const { return m_title; }

protected:
    RenderTarget    m_renderTarget;
    Renderer        m_renderer;
    const int       m_width;
    const int       m_height;
    std::string     m_title;
};

extern BaseApp* g_pApp;

} // namespace hguo
