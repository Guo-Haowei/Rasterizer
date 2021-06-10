#pragma once
#include "Texture.h"

namespace hguo {

class RenderTarget {
   public:
    struct CreateInfo {
        int width;
        int height;
        bool useColor;
        bool useDepth;
    };

    void create(const CreateInfo& info);

    void resize(int width, int height);

    inline const ColorBuffer& getColorBuffer() const { return m_colorBuffer; }

    inline const DepthBuffer& getDepthBuffer() const { return m_depthBuffer; }

   public:
    ColorBuffer m_colorBuffer;
    DepthBuffer m_depthBuffer;

    bool m_useColor = true;
    bool m_useDepth = true;
};

}  // namespace hguo
