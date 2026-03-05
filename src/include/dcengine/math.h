#pragma once
#include <raylib.h>
#include <cmath>

template<typename T>
inline T sqr(T a) { return a * a; }

template<typename T>
struct vec2
{
  T x, y;

  vec2() = default;
  vec2(const vec2<T>& rhs) = default;
  vec2(vec2<T>&& rhs) = default;
  vec2(T _x, T _y) : x(_x), y(_y) {}

  vec2<T>& operator=(const vec2<T>& rhs) = default;
  vec2<T>& operator=(vec2<T>&& rhs) = default;

  vec2<T> operator-() const { return vec2<T>(-x, -y); }

  T dot(const vec2<T>& rhs) const { return x * rhs.x + y * rhs.y; }

  T mag2() const { return sqr(x) + sqr(y); }
  T mag() const { return sqrtf(mag2()); }

  void normalize()
  {
    T len = mag();
    x /= len;
    y /= len;
  }

  vec2<T>& operator*=(T rhs)
  {
    x *= rhs;
    y *= rhs;
    return *this;
  }

  vec2<T> normalized() const
  {
    T len = mag();
    return vec2<T>(x / len, y / len);
  }

  bool operator==(const vec2<T>& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator!=(const vec2<T>& rhs) const { return x != rhs.x || y != rhs.y; }

  vec2<T>& operator+=(const vec2<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  vec2<T>& operator-=(const vec2<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }
};

template<typename T>
inline vec2<T> truncate(const vec2<T> &v, float len)
{
  const float l2 = v.mag2();
  if (l2 > sqr(len))
    return v * (len / sqrtf(l2));
  return v;
}


template<typename T>
struct vec3
{
  T x, y, z;

  vec3() = default;
  vec3(const vec3<T>& rhs) = default;
  vec3(vec3<T>&& rhs) = default;
  vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

  vec3<T>& operator=(const vec3<T>& rhs) = default;
  bool operator==(const vec3<T>& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

  vec3<T> operator-() const { return vec3<T>(-x, -y, -z); }

  T dot(const vec3<T>& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

  vec3<T>& operator+=(const vec3<T>& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
  vec3<T>& operator-=(const vec3<T>& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  T mag2() const { return sqr(x) + sqr(y) + sqr(z); }
  T mag() const { return sqrtf(mag2()); }

  void normalize()
  {
    T len = mag();
    x /= len;
    y /= len;
    z /= len;
  }
  vec3<T> normalized() const
  {
    T len = mag();
    return vec3<T>(x / len, y / len, z / len);
  }

  vec2<T> xy() const { return vec2<T>(x, y); }
  vec2<T> xz() const { return vec2<T>(x, z); }
  vec2<T> yz() const { return vec2<T>(y, z); }
};

template<typename T>
struct vec4
{
  T x, y, z, w;

  vec4() = default;
  vec4(const vec4<T>& rhs) = default;
  vec4(vec4<T>&& rhs) = default;
  vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

  vec4<T>& operator=(const vec4<T>& rhs) = default;
};


template<typename T>
inline vec3<T> operator+(const vec3<T>& lhs, const vec3<T>& rhs) { return vec3<T>{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }

template<typename T>
inline vec3<T> operator-(const vec3<T>& lhs, const vec3<T>& rhs) { return vec3<T>{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }

template<typename T>
inline vec3<T> operator*(const vec3<T>& lhs, const T& rhs) { return vec3<T>{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs}; }

template<typename T>
inline vec3<T> operator*(const T& lhs, const vec3<T>& rhs) { return vec3<T>{lhs * rhs.x, lhs * rhs.y, lhs * rhs.z}; }

template<typename T>
inline vec3<T> operator*(const vec3<T>& lhs, const vec3<T>& rhs) { return vec3<T>{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z}; }

template<typename T>
inline vec2<T> operator+(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>{lhs.x + rhs.x, lhs.y + rhs.y}; }

template<typename T>
inline vec2<T> operator-(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>{lhs.x - rhs.x, lhs.y - rhs.y}; }

template<typename T>
inline vec2<T> operator*(const vec2<T>& lhs, const T& rhs) { return vec2<T>{lhs.x * rhs, lhs.y * rhs}; }

template<typename T>
inline vec2<T> operator*(const T& lhs, const vec2<T>& rhs) { return vec2<T>{lhs * rhs.x, lhs * rhs.y}; }

template<typename T>
inline bool operator<(const vec2<T>& lhs, const vec2<T>& rhs) { return lhs.x < rhs.x ? true : lhs.x == rhs.x ? lhs.y < rhs.y : false; }

template<typename T>
inline bool operator<(const vec3<T>& lhs, const vec3<T>& rhs)
{
    // TODO: make it pretty...
    if (lhs.x < rhs.x)
        return true;
    else if (lhs.x > rhs.x)
        return false;
    else if (lhs.y < rhs.y)
        return true;
    else if (lhs.y > rhs.y)
        return false;
    else
        return lhs.z < rhs.z;
}


using vec2f = vec2<float>;
using vec2i = vec2<int>;
using vec3f = vec3<float>;
using vec3i = vec3<int>;
using vec4f = vec4<float>;
using vec4i = vec4<int>;

inline Vector2 toRLVec2(const vec2f& vec) { return Vector2{vec.x, vec.y}; }
inline Vector3 toRLVec3(const vec3f& vec) { return Vector3{vec.x, vec.y, vec.z}; }
inline Vector4 toRLVec4(const vec4f& vec) { return Vector4{vec.x, vec.y, vec.z, vec.w}; }
inline const Vector3& castRLVec3(const vec3f& vec) { return *reinterpret_cast<const Vector3*>(&vec); }

template<typename T1, typename T2>
inline Vector2 tovec(T1 p1, T2 p2)
{
  return Vector2{float(p1), float(p2)};
}

template<typename T, typename U, typename V>
inline Vector3 tovec3(T x, U y, V z) { return Vector3{float(x), float(y), float(z)}; }
inline vec2f tov2(const Vector2& v) { return vec2f{v.x, v.y}; }
inline vec3f tov3(const Vector3& v) { return vec3f{v.x, v.y, v.z}; }

// in cmath inline float lerp(float a, float b, float t) { return a * (1.f - t) + b * t; }
inline vec3f lerp(vec3f a, vec3f b, float t) { return a * (1.f - t) + b * t; }

void register_math(eecs::Registry& reg);

template<typename T>
T sign(T v) { return v > 0 ? 1 : v < 0 ? -1 : 0; }

template<typename T1, typename T2, typename T3, typename T4>
inline Rectangle torect(T1 p1, T2 p2, T3 p3, T4 p4)
{
  return Rectangle{float(p1), float(p2), float(p3), float(p4)};
}

inline float clamp(float v, float min, float max)
{
  return v < min ? min : v > max ? max : v;
}

inline float point_to_segm_dist(const vec2f& pt, const vec2f& start, const vec2f& end)
{
  vec2f dir = end - start;
  float segmLen = dir.mag();
  dir *= 1.f / segmLen; // TODO check 0
  vec2f toPt = pt - start;
  float dp = toPt.dot(dir);
  float dist = (pt - (start + dir * clamp(dp, 0.f, segmLen))).mag();
  return dist;
}

inline void line_ray_closest(const vec3f& p1, const vec3f& p2, const vec3f& ray_start, const vec3f& dir, float& mua, float& mub)
{
  vec3f p13 = p1 - ray_start;
  vec3f p21 = p2 - p1;

  float denom = p21.mag2() - sqr(dir.dot(p21));
  float numer = p13.dot(dir) * dir.dot(p21) - p13.dot(p21);

  constexpr float eps = 1e-7f;

  mua = denom > eps ? numer / denom : denom;
  mub = p13.dot(dir) + dir.dot(p21) * mua;
}

inline float move_to(float from, float to, float dt, float vel)
{
  float d = vel * dt;
  if (fabsf(from - to) < d)
    return to;

  if (to < from)
    return from - d;
  else
    return from + d;
}

template <typename T>
inline T move_to_vec(const T &from, const T &to, float dt, float vel)
{
  float d = vel * dt;
  T dir = to - from;
  float dirLenSq = dir.mag2();
  if (dirLenSq <= sqr(d))
    return to;

  return from + dir * (d / sqrtf(dirLenSq));
}

inline vec2i pos_to_grid(const vec3f& pos)
{
  return vec2i{(int)floorf(pos.x + 0.5f), (int)floorf(pos.z + 0.5f)};
}

inline vec3i pos_to_grid3d(const vec3f& pos)
{
  return vec3i{(int)roundf(pos.x), (int)roundf(pos.y), (int)roundf(pos.z)};
}


