#pragma once

#include <DirectXMath.h>
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
    namespace Math
    {
        class Quaternioni
        {

        public:

            Quaternioni() : x(0), y(0), z(0), w(0) {}
            Quaternioni(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
            Quaternioni(int scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Quaternioni(const Quaternioni& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Quaternioni(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Quaternioni& operator=(const Quaternioni& other)
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

            Quaternioni& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = static_cast<int>(temp.x);
                y = static_cast<int>(temp.y);
                z = static_cast<int>(temp.z);
                w = static_cast<int>(temp.w);

                return *this;
            }

            Quaternioni operator+(const Quaternioni& other) const
            {
                return Quaternioni(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Quaternioni operator-(const Quaternioni& other) const
            {
                return Quaternioni(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Quaternioni operator*(const Quaternioni& other) const
            {
                return Quaternioni(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Quaternioni operator/(const Quaternioni& other) const
            {
                return Quaternioni(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Quaternioni operator+(int scalar) const
            {
                return Quaternioni(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Quaternioni operator-(int scalar) const
            {
                return Quaternioni(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Quaternioni operator*(int scalar) const
            {
                return Quaternioni(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Quaternioni operator/(int scalar) const
            {
                return Quaternioni(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Quaternioni& operator+=(const Quaternioni& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Quaternioni& operator-=(const Quaternioni& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Quaternioni& operator*=(const Quaternioni& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Quaternioni& operator/=(const Quaternioni& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Quaternioni& operator+=(int scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Quaternioni& operator-=(int scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Quaternioni& operator*=(int scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Quaternioni& operator/=(int scalar)
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

        class Quaternionf
        {

        public:

            Quaternionf() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
            Quaternionf(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
            Quaternionf(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Quaternionf(const Quaternionf& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Quaternionf(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Quaternionf& operator=(const Quaternionf& other)
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

            Quaternionf& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = temp.x;
                y = temp.y;
                z = temp.z;
                w = temp.w;
                return *this;
            }

            Quaternionf operator+(const Quaternionf& other) const
            {
                return Quaternionf(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Quaternionf operator-(const Quaternionf& other) const
            {
                return Quaternionf(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Quaternionf operator*(const Quaternionf& other) const
            {
                return Quaternionf(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Quaternionf operator/(const Quaternionf& other) const
            {
                return Quaternionf(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Quaternionf operator+(float scalar) const
            {
                return Quaternionf(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Quaternionf operator-(float scalar) const
            {
                return Quaternionf(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Quaternionf operator*(float scalar) const
            {
                return Quaternionf(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Quaternionf operator/(float scalar) const
            {
                return Quaternionf(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Quaternionf& operator+=(const Quaternionf& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Quaternionf& operator-=(const Quaternionf& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Quaternionf& operator*=(const Quaternionf& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Quaternionf& operator/=(const Quaternionf& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Quaternionf& operator+=(float scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Quaternionf& operator-=(float scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Quaternionf& operator*=(float scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Quaternionf& operator/=(float scalar)
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

        class Quaterniond
        {

        public:

            Quaterniond() : x(0.0), y(0.0), z(0.0), w(0.0) {}
            Quaterniond(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
            Quaterniond(double scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Quaterniond(const Quaterniond& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Quaterniond(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Quaterniond& operator=(const Quaterniond& other)
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

            Quaterniond& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT4 temp;
                DirectX::XMStoreFloat4(&temp, other);

                x = temp.x;
                y = temp.y;
                z = temp.z;
                w = temp.w;

                return *this;
            }

            Quaterniond operator+(const Quaterniond& other) const
            {
                return Quaterniond(x + other.x, y + other.y, z + other.z, w + other.w);
            }

            Quaterniond operator-(const Quaterniond& other) const
            {
                return Quaterniond(x - other.x, y - other.y, z - other.z, w - other.w);
            }

            Quaterniond operator*(const Quaterniond& other) const
            {
                return Quaterniond(x * other.x, y * other.y, z * other.z, w * other.w);
            }

            Quaterniond operator/(const Quaterniond& other) const
            {
                return Quaterniond(x / other.x, y / other.y, z / other.z, w / other.w);
            }

            Quaterniond operator+(double scalar) const
            {
                return Quaterniond(x + scalar, y + scalar, z + scalar, w + scalar);
            }

            Quaterniond operator-(double scalar) const
            {
                return Quaterniond(x - scalar, y - scalar, z - scalar, w - scalar);
            }

            Quaterniond operator*(double scalar) const
            {
                return Quaterniond(x * scalar, y * scalar, z * scalar, w * scalar);
            }

            Quaterniond operator/(double scalar) const
            {
                return Quaterniond(x / scalar, y / scalar, z / scalar, w / scalar);
            }

            Quaterniond& operator+=(const Quaterniond& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;

                return *this;
            }

            Quaterniond& operator-=(const Quaterniond& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;

                return *this;
            }

            Quaterniond& operator*=(const Quaterniond& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;
                w *= other.w;

                return *this;
            }

            Quaterniond& operator/=(const Quaterniond& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;
                w /= other.w;

                return *this;
            }

            Quaterniond& operator+=(double scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;
                w += scalar;

                return *this;
            }

            Quaterniond& operator-=(double scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;
                w -= scalar;

                return *this;
            }

            Quaterniond& operator*=(double scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;

                return *this;
            }

            Quaterniond& operator/=(double scalar)
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