#pragma once
#define GLM_FORCE_SSE2  // or GLM_FORCE_SSE42
#define GLM_FORCE_ALIGNED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "common/core_assert.h"

namespace gfx {

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::cross;
using glm::dot;
using glm::max;
using glm::min;
using glm::mix;
using glm::normalize;
using glm::slerp;

using glm::lookAt;
using glm::perspectiveRH_NO;
using glm::rotate;
using glm::scale;
using glm::translate;

using glm::toMat4;

struct Color {
    unsigned char r, g, b, a;
};

template <int N, typename T>
struct box_t {
    using vec_t = glm::vec<N, T, glm::defaultp>;
    using this_t = box_t<N, T>;

    vec_t min, max;

    constexpr box_t()
        : min { std::numeric_limits<T>::infinity() }
        , max { -std::numeric_limits<T>::infinity() } {}

    constexpr box_t(const vec_t& _min, const vec_t& _max)
        : min(_min)
        , max(_max) {
        ASSERT(isValid());
    }

    void expandPoint(const vec_t& p) {
        min = gfx::min(min, p);
        max = gfx::max(max, p);
    }

    void unionBox(const this_t& o) {
        min = gfx::max(min, o.min);
        max = gfx::min(max, o.max);
    }

    bool isValid() const {
        bool valid = (min.x < max.x) && (min.y < max.y);
        if constexpr (N > 2) {
            valid = valid && (min.z < max.z);
        }
        if constexpr (N > 3) {
            valid = valid && (min.w < max.w);
        }
        return valid;
    }
};

using Box2 = box_t<2, float>;
using Box3 = box_t<3, float>;

}  // namespace gfx
