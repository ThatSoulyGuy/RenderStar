#pragma once

#include <DirectXMath.h>
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
    namespace Math
    {
        class Vector4i
        {

        public:

            Vector4i() : x(0), y(0), z(0), w(0) {}
            Vector4i(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
            Vector4i(int scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Vector4i(const Vector4i& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Vector4i(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Vector4i& operator=(const Vector4i& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                    z = other.z;
                    w = other.w;
                }

                return *this;
            }

            Vector4i& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = static_cast<int>(temp.x);
                y = static_cast<int>(temp.y);
                z = static_cast<int>(temp.z);
                w = static_cast<int>(temp.w);

                return *this;
            }

            Vector4i operator+(const Vector4i& other) const
            {
                return Vector4i(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Vector4i operator-(const Vector4i& other) const
            {
                return Vector4i(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Vector4i operator*(const Vector4i& other) const
            {
                return Vector4i(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Vector4i operator/(const Vector4i& other) const
            {
                return Vector4i(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Vector4i operator+(int scalar) const
            {
                return Vector4i(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Vector4i operator-(int scalar) const
            {
                return Vector4i(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Vector4i operator*(int scalar) const
            {
                return Vector4i(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Vector4i operator/(int scalar) const
            {
                return Vector4i(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Vector4i& operator+=(const Vector4i& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Vector4i& operator-=(const Vector4i& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Vector4i& operator*=(const Vector4i& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Vector4i& operator/=(const Vector4i& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Vector4i& operator+=(int scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Vector4i& operator-=(int scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Vector4i& operator*=(int scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Vector4i& operator/=(int scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
                w /= scalar;

                return *this;
            }

            operator DirectX::XMINT2() const
            {
                return DirectX::XMINT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, w);
            }

            operator DirectX::XMINT3() const
            {
                return DirectX::XMINT3(x, y, z);
            }

            operator DirectX::XMINT4() const
            {
                return DirectX::XMINT4(x, y, z, w);
            }

            int x = 0;
            int y = 0;
            int z = 0;
            int w = 0;
        };

        class Vector4f
        {

        public:

            Vector4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
            Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
            Vector4f(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Vector4f(const Vector4f& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Vector4f(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Vector4f& operator=(const Vector4f& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                    z = other.z;
                    w = other.w;
                }

                return *this;
            }

            Vector4f& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = temp.x;
                y = temp.y;
                z = temp.z;
                w = temp.w;
                return *this;
            }

            Vector4f operator+(const Vector4f& other) const
            {
                return Vector4f(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Vector4f operator-(const Vector4f& other) const
            {
                return Vector4f(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Vector4f operator*(const Vector4f& other) const
            {
                return Vector4f(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Vector4f operator/(const Vector4f& other) const
            {
                return Vector4f(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Vector4f operator+(float scalar) const
            {
                return Vector4f(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Vector4f operator-(float scalar) const
            {
                return Vector4f(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Vector4f operator*(float scalar) const
            {
                return Vector4f(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Vector4f operator/(float scalar) const
            {
                return Vector4f(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Vector4f& operator+=(const Vector4f& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Vector4f& operator-=(const Vector4f& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Vector4f& operator*=(const Vector4f& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Vector4f& operator/=(const Vector4f& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Vector4f& operator+=(float scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Vector4f& operator-=(float scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Vector4f& operator*=(float scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Vector4f& operator/=(float scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
                w /= scalar;

                return *this;
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, w);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3(x, y, z);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4(x, y, z, w);
            }

            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            float w = 0.0f;
        };

        class Vector4d
        {

        public:

            Vector4d() : x(0.0), y(0.0), z(0.0), w(0.0) {}
            Vector4d(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
            Vector4d(double scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Vector4d(const Vector4d& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Vector4d(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Vector4d& operator=(const Vector4d& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                    z = other.z;
                    w = other.w;
                }
                return *this;
            }

            Vector4d& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = temp.x;
                y = temp.y;
                z = temp.z;
                w = temp.w;

                return *this;
            }

            Vector4d operator+(const Vector4d& other) const
            {
                return Vector4d(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Vector4d operator-(const Vector4d& other) const
            {
                return Vector4d(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Vector4d operator*(const Vector4d& other) const
            {
                return Vector4d(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Vector4d operator/(const Vector4d& other) const
            {
                return Vector4d(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Vector4d operator+(double scalar) const
            {
                return Vector4d(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Vector4d operator-(double scalar) const
            {
                return Vector4d(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Vector4d operator*(double scalar) const
            {
                return Vector4d(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Vector4d operator/(double scalar) const
            {
                return Vector4d(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Vector4d& operator+=(const Vector4d& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Vector4d& operator-=(const Vector4d& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Vector4d& operator*=(const Vector4d& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Vector4d& operator/=(const Vector4d& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Vector4d& operator+=(double scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Vector4d& operator-=(double scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Vector4d& operator*=(double scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Vector4d& operator/=(double scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
                w /= scalar;

                return *this;
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, w);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3(x, y, z);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4(x, y, z, w);
            }

            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            double w = 0.0;
        };
    }
}