#include "Maths.h"
#include <assert.h>

namespace hguo {

/**
 * P = uA + vB + wC (where w = 1 - u - v)
 * P = uA + vB + C - uC - vC => (C - P) + u(A - C) + v(B - C) = 0
 * uCA + vCB + PC = 0
 * [u v 1] [CAx CBx PCx] = 0
 * [u v 1] [CAy CBy PCy] = 0
 * [u v 1] is the cross product
 */
Vector3 barycentric(const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& p)
{
    const Vector2 PC = c - p, CA = a - c, CB = b - c;
    Vector3 uvw = three::cross(Vector3(CA.x, CB.x, PC.x), Vector3(CA.y, CB.y, PC.y));
    // vec3 uvw = glm::cross(vec3(CA.x, CB.x, PC.x), vec3(CA.y, CB.y, PC.y));
    /**
     * what if uvw.z is 0?
     * CA.x * CB.y = CA.y * CB.x == 0 => CA.y / CA.x == CB.y / CB.x
     * CA // CB => A, B and C are on the same line
     */
#ifdef _DEBUG
    // we probably want to discard the triangle before rendering
    assert(uvw.z != 0.0f);
#endif
    uvw /= uvw.z;
    uvw.z -= (uvw.x + uvw.y);
    return uvw;
}

} // namespace hguo
