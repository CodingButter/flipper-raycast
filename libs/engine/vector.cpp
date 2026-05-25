#include "vector.h"
#include <math.h>

// ---- Vec2 ----

float Vec2::length() const {
    return sqrtf(x * x + y * y);
}

float Vec2::distance_to(const Vec2& o) const {
    return sqrtf(sqr_distance_to(o));
}

Vec2& Vec2::normalize() {
    float len = length();
    if (len > 0.0f) {
        x /= len;
        y /= len;
    } else {
        x = 0.0f;
        y = 0.0f;
    }
    return *this;
}

Vec2& Vec2::rotate(float angle_rad) {
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float nx = x * c - y * s;
    float ny = x * s + y * c;
    x = nx;
    y = ny;
    return *this;
}

// ---- Vec3 ----

float Vec3::length() const {
    return sqrtf(x * x + y * y + z * z);
}

float Vec3::distance_to(const Vec3& o) const {
    return sqrtf(sqr_distance_to(o));
}

Vec3& Vec3::normalize() {
    float len = length();
    if (len > 0.0f) {
        x /= len;
        y /= len;
        z /= len;
    } else {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }
    return *this;
}

Vec3& Vec3::cross(const Vec3& o) {
    float nx = y * o.z - z * o.y;
    float ny = z * o.x - x * o.z;
    float nz = x * o.y - y * o.x;
    x = nx;
    y = ny;
    z = nz;
    return *this;
}
