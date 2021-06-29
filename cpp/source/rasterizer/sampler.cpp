#include "Sampler.h"

namespace rs {

using namespace gfx;
template <>
const Color TextureBase<Color>::sDefaultValue { 0, 0, 0, 255 };
template <>
const float TextureBase<float>::sDefaultValue { 1.0f };

}  // namespace rs
