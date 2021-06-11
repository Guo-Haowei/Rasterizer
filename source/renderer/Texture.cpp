#include "Texture.h"

namespace hguo {

using namespace gfx;
template <>
const Color TextureBase<Color>::sDefaultValue { 0, 0, 0, 255 };
template <>
const float TextureBase<float>::sDefaultValue { 1.0f };

}  // namespace hguo
