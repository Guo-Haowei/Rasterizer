#pragma once
#include <string>
#include "rasterizer/rasterizer.h"

struct Config {
    int width;
    int height;
    const char* title;
};

class ExampleBase {
   public:
    ExampleBase(const Config& config);

    virtual void initialize() = 0;

    virtual void update(double deltaTime) = 0;

    virtual void finalize() = 0;

    virtual const rs::Texture& getTexture() const { return m_renderTarget.m_colorBuffer; }

    int getWidth() const { return m_width; }

    int getHeight() const { return m_height; }

    const std::string& getTitle() const { return m_title; }

   protected:
    rs::RenderTarget m_renderTarget;
    rs::Renderer m_renderer;
    const int m_width;
    const int m_height;
    std::string m_title;
};

extern Config g_config;
extern ExampleBase* g_pExample;
