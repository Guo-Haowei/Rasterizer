#pragma once
#include "Types.h"
#include "Vector.h"
#include <limits>
#include <assert.h>

namespace three {

template<int M, typename T> struct box_type;

using Box2  = box_type<2, real>;
using Box3  = box_type<3, real>;

template<int M, typename T> struct box_type
{
    using vec_type = linmath::vector_type<M, T>;

    vec_type min;
    vec_type max;

    constexpr box_type();

    constexpr box_type(const vec_type& min, const vec_type& max);

    void expandPoint(const vec_type& point);

    void unionBox(const box_type<M, T>& other);

    bool isValid() const;
};

template<int M, typename T>
constexpr box_type<M, T>::box_type()
    : min { std::numeric_limits<T>::infinity() }, max { -std::numeric_limits<T>::infinity() }
{}

template<int M, typename T>
constexpr box_type<M, T>::box_type(const vec_type& _min, const vec_type& _max)
    : min(_min), max(_max)
{
    assert(_min <= _max);
}


template<int M, typename T>
void box_type<M, T>::expandPoint(const vec_type& point)
{
    min = linmath::min(min, point);
    max = linmath::max(max, point);
}

template<int M, typename T>
void box_type<M, T>::unionBox(const box_type<M, T>& other)
{
    min = linmath::max(min, other.min);
    max = linmath::min(max, other.max);
}

template<int M, typename T>
bool box_type<M, T>::isValid() const
{
    return min < max;
}

} // namespace three
