// ---------------------------------------------------------------------------
// Vec2 / Vec3 — three.js-style vector math.
//
// All mutators (add, sub, mul, normalize, ...) modify *this in place AND
// return a reference, so you can chain:
//
//     velocity.add(gravity).mul(damping);
//     ray_dir.copy(facing).rotate(angle_offset);
//
// Use clone() to get an independent copy, copy(other) to overwrite this
// vector's components with another's:
//
//     Vec2 saved = pos.clone();   // snapshot
//     pos.copy(target);           // teleport
//
// Distance / length come in two flavors:
//
//     length(), distance_to()       → uses sqrtf (slower, real units)
//     sqr_length(), sqr_distance_to() → no sqrt (fast, use for comparisons)
// ---------------------------------------------------------------------------

#pragma once

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}

    // ---- Mutators (return *this for chaining) ----
    Vec2& set(float x, float y)      { this->x = x; this->y = y; return *this; }
    Vec2& copy(const Vec2& o)        { x = o.x; y = o.y; return *this; }
    Vec2& add(const Vec2& o)         { x += o.x; y += o.y; return *this; }
    Vec2& sub(const Vec2& o)         { x -= o.x; y -= o.y; return *this; }
    Vec2& mul(float s)               { x *= s;  y *= s;  return *this; }
    Vec2& div(float s)               { x /= s;  y /= s;  return *this; }
    Vec2& negate()                   { x = -x;  y = -y;  return *this; }
    Vec2& normalize();               // sets to (0,0) if zero-length
    Vec2& rotate(float angle_rad);   // 2D rotation about origin

    // ---- Producers (return new Vec2) ----
    Vec2 clone() const               { return {x, y}; }

    // ---- Read-only queries ----
    constexpr float sqr_length()             const { return x * x + y * y; }
    float           length()                const;
    constexpr float dot(const Vec2& o)      const { return x * o.x + y * o.y; }
    constexpr float sqr_distance_to(const Vec2& o) const {
        float dx = x - o.x, dy = y - o.y;
        return dx * dx + dy * dy;
    }
    float           distance_to(const Vec2& o) const;
    constexpr bool  equals(const Vec2& o)   const { return x == o.x && y == o.y; }

    // ---- Operator overloads (return new Vec2) ----
    constexpr Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator-()              const { return {-x, -y}; }
    constexpr Vec2 operator*(float s)       const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s)       const { return {x / s, y / s}; }

    // Compound assignment — modify in place, return reference for chaining.
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s)       { x *= s;  y *= s;  return *this; }
    Vec2& operator/=(float s)       { x /= s;  y /= s;  return *this; }

    constexpr bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    constexpr bool operator!=(const Vec2& o) const { return !(*this == o); }
};

// Allow `2.0f * v` to read naturally as well as `v * 2.0f`.
constexpr Vec2 operator*(float s, const Vec2& v) { return v * s; }

// Linear interpolation: t=0 → a, t=1 → b, in between blends.
// Values outside [0, 1] extrapolate; that's intentional.
constexpr Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
    return a + (b - a) * t;
}

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // ---- Mutators ----
    Vec3& set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; return *this; }
    Vec3& copy(const Vec3& o)            { x = o.x; y = o.y; z = o.z; return *this; }
    Vec3& add(const Vec3& o)             { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& sub(const Vec3& o)             { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& mul(float s)                   { x *= s;  y *= s;  z *= s;  return *this; }
    Vec3& div(float s)                   { x /= s;  y /= s;  z /= s;  return *this; }
    Vec3& negate()                       { x = -x;  y = -y;  z = -z;  return *this; }
    Vec3& normalize();
    Vec3& cross(const Vec3& o);          // in-place cross-product

    // ---- Producers ----
    Vec3 clone() const                   { return {x, y, z}; }

    // ---- Read-only queries ----
    constexpr float sqr_length()              const { return x * x + y * y + z * z; }
    float           length()                 const;
    constexpr float dot(const Vec3& o)       const { return x * o.x + y * o.y + z * o.z; }
    constexpr float sqr_distance_to(const Vec3& o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return dx * dx + dy * dy + dz * dz;
    }
    float           distance_to(const Vec3& o) const;
    constexpr bool  equals(const Vec3& o)    const { return x == o.x && y == o.y && z == o.z; }

    // ---- Operator overloads (mirror Vec2) ----
    constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3 operator-()              const { return {-x, -y, -z}; }
    constexpr Vec3 operator*(float s)       const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator/(float s)       const { return {x / s, y / s, z / s}; }

    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s)       { x *= s;  y *= s;  z *= s;  return *this; }
    Vec3& operator/=(float s)       { x /= s;  y /= s;  z /= s;  return *this; }

    constexpr bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    constexpr bool operator!=(const Vec3& o) const { return !(*this == o); }
};

constexpr Vec3 operator*(float s, const Vec3& v) { return v * s; }

constexpr Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return a + (b - a) * t;
}
