#pragma once
#include <limits>
#define THREE_MATH_DISABLE_SWIZZLE
#include <iostream>
#include "include/Box.h"
#include "include/Matrix.h"
#include "include/Quaternion.h"
#include "include/Vector.h"

using namespace three;

namespace hguo {

// using three::Vector4i;
// using three::Vector2;
// using three::Vector3;
// using three::Vector4;
// using three::Matrix4;
// using three::Box2;
// using three::Quaternion;

using Color = linmath::vector4_type<unsigned char>;

extern Vector3 barycentric(const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& p);

}  // namespace hguo
