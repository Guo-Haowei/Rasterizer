#pragma once
#include <algorithm>  // std::fill
#include <vector>
#include "common/core_assert.h"
#include "common/linalg.h"

namespace hguo {

template <class T>
class TextureBase {
   public:
    struct CreateInfo {
        int width;
        int height;
        const void* data;
    };

    void create(const CreateInfo& info) {
        ASSERT(info.width != 0 && info.height != 0);
        m_width = info.width;
        m_height = info.height;
        m_buffer.resize(m_width * m_height);
        memcpy(&m_buffer[0], info.data, sizeof(T) * m_width * m_height);
    }

    const T& sample(gfx::vec2 uv) const {
        int x = static_cast<int>(uv.x * m_width);
        int y = static_cast<int>(uv.y * m_height);
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return sDefaultValue;
        } else {
            return m_buffer[y * m_width + x];
        }
    }

    void resize(int width, int height) {
        m_width = width;
        m_height = height;
        m_buffer.resize(m_width * m_height);
    }

    void clear(const T& clearValue) {
        std::fill(m_buffer.begin(), m_buffer.end(), clearValue);
    }

    const void* getData() const { return m_buffer.data(); }

   public:
    int m_width = 0;
    int m_height = 0;
    std::vector<T> m_buffer;

    static const T sDefaultValue;
};

typedef TextureBase<gfx::Color> Texture;
typedef TextureBase<float> DepthBuffer;
typedef Texture ColorBuffer;

}  // namespace hguo
