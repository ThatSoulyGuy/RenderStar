#pragma once

#include <DirectXMath.h>
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
    namespace Math
    {
        class Vector3i
        {

        public:

            Vector3i() : x(0), y(0), z(0) {}
            Vector3i(int x, int y, int z) : x(x), y(y), z(z) {}
            Vector3i(int scalar) : x(scalar), y(scalar), z(scalar) {}
            Vector3i(const Vector3i& other) : x(other.x), y(other.y), z(other.z) {}

            Vector3i(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), vector);
            }

            Vector3i& operator=(const Vector3i& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                    z = other.z;
                }

                return *this;
            }

            Vector3i& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT3 temp;

                DirectX::XMStoreFloat3(&temp, other);

                x = static_cast<int>(temp.x);
                y = static_cast<int>(temp.y);
                z = static_cast<int>(temp.z);

                return *this;
            }

            Vector3i operator+(const Vector3i& other) const
            {
                return Vector3i(x + other.x, y + other.y, z + other.z);
            }

            Vector3i operator-(const Vector3i& other) const
            {
                return Vector3i(x - other.x, y - other.y, z - other.z);
            }

            Vector3i operator*(const Vector3i& other) const
            {
                return Vector3i(x * other.x, y * other.y, z * other.z);
            }

            Vector3i operator/(const Vector3i& other) const
            {
                return Vector3i(x / other.x, y / other.y, z / other.z);
            }

            Vector3i operator+(int scalar) const
            {
                return Vector3i(x + scalar, y + scalar, z + scalar);
            }

            Vector3i operator-(int scalar) const
            {
                return Vector3i(x - scalar, y - scalar, z - scalar);
            }

            Vector3i operator*(int scalar) const
            {
                return Vector3i(x * scalar, y * scalar, z * scalar);
            }

            Vector3i operator/(int scalar) const
            {
                return Vector3i(x / scalar, y / scalar, z / scalar);
            }

            Vector3i& operator+=(const Vector3i& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;

                return *this;
            }

            Vector3i& operator-=(const Vector3i& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;

                return *this;
            }

            Vector3i& operator*=(const Vector3i& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;

                return *this;
            }

            Vector3i& operator/=(const Vector3i& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;

                return *this;
            }

            Vector3i& operator+=(int scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;

                return *this;
            }

            Vector3i& operator-=(int scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;

                return *this;
            }

            Vector3i& operator*=(int scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;

                return *this;
            }

            Vector3i& operator/=(int scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;

                return *this;
            }

            operator DirectX::XMINT2() const
            {
                return DirectX::XMINT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, 0.0f);
            }

            operator DirectX::XMINT3() const
            {
                return DirectX::XMINT3(x, y, z);
            }

            operator DirectX::XMINT4() const
            {
                return DirectX::XMINT4(x, y, z, 0.0f);
            }

            int x = 0;
            int y = 0;
            int z = 0;
        };

        bool operator==(const Vector3i& lhs, const Vector3i& rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        bool operator!=(const Vector3i& lhs, const Vector3i& rhs)
        {
            return !(lhs == rhs);
        }

        class Vector3f
        {

        public:

            Vector3f() : x(0.0f), y(0.0f), z(0.0f) {}
            Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
            Vector3f(float scalar) : x(scalar), y(scalar), z(scalar) {}
            Vector3f(const Vector3f& other) : x(other.x), y(other.y), z(other.z) {}

            Vector3f(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), vector);
            }

            Vector3f& operator=(const Vector3f& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                    z = other.z;
                }

                return *this;
            }

            Vector3f& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT3 temp;

                DirectX::XMStoreFloat3(&temp, other);

                x = static_cast<int>(temp.x);
                y = static_cast<int>(temp.y);
                z = static_cast<int>(temp.z);

                return *this;
            }

            Vector3f operator+(const Vector3f& other) const
            {
                return Vector3f(x + other.x, y + other.y, z + other.z);
            }

            Vector3f operator-(const Vector3f& other) const
            {
                return Vector3f(x - other.x, y - other.y, z - other.z);
            }

            Vector3f operator*(const Vector3f& other) const
            {
                return Vector3f(x * other.x, y * other.y, z * other.z);
            }

            Vector3f operator/(const Vector3f& other) const
            {
                return Vector3f(x / other.x, y / other.y, z / other.z);
            }

            Vector3f operator+(float scalar) const
            {
                return Vector3f(x + scalar, y + scalar, z + scalar);
            }

            Vector3f operator-(float scalar) const
            {
                return Vector3f(x - scalar, y - scalar, z - scalar);
            }

            Vector3f operator*(float scalar) const
            {
                return Vector3f(x * scalar, y * scalar, z * scalar);
            }

            Vector3f operator/(float scalar) const
            {
                return Vector3f(x / scalar, y / scalar, z / scalar);
            }

            Vector3f& operator+=(const Vector3f& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;

                return *this;
            }

            Vector3f& operator-=(const Vector3f& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;

                return *this;
            }

            Vector3f& operator*=(const Vector3f& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;

                return *this;
            }

            Vector3f& operator/=(const Vector3f& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;

                return *this;
            }

            Vector3f& operator+=(float scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;

                return *this;
            }

            Vector3f& operator-=(float scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;

                return *this;
            }

            Vector3f& operator*=(float scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;

                return *this;
            }

            Vector3f& operator/=(float scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;

                return *this;
            }

            float Length() const
            {
                return sqrt(x * x + y * y + z * z);
            }

            Vector3f Normalized() const
            {
                float length = Length();

                if (length == 0.0f)
                    return Vector3f(0.0f, 0.0f, 0.0f);
                
                return Vector3f(x / length, y / length, z / length);
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, 0.0f);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3(x, y, z);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4(x, y, z, 0.0f);
            }

            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };

        bool operator==(const Vector3f& lhs, const Vector3f& rhs)
		{
			return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
		}

        bool operator!=(const Vector3f& lhs, const Vector3f& rhs)
        {
            return !(lhs == rhs);
        }

        Vector3f operator*(float scalar, const Vector3f& vector)
        {
            return vector * scalar;
        }

        class Vector3d
        {

        public:

            Vector3d() : x(0.0), y(0.0), z(0.0) {}
            Vector3d(double x, double y, double z) : x(x), y(y), z(z) {}
            Vector3d(double scalar) : x(scalar), y(scalar), z(scalar) {}
            Vector3d(const Vector3d& other) : x(other.x), y(other.y), z(other.z) {}

            Vector3d(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(this), vector);
            }

            Vector3d& operator=(const Vector3d& other)
            {
                if (this != &other)
                    x = other.x;
                y = other.y;
                z = other.z;

                return *this;
            }

            Vector3d& operator=(const DirectX::XMVECTOR& other)
            {
                DirectX::XMFLOAT3 temp;

                DirectX::XMStoreFloat3(&temp, other);

                x = static_cast<int>(temp.x);
                y = static_cast<int>(temp.y);
                z = static_cast<int>(temp.z);

                return *this;
            }

            Vector3d operator+(const Vector3d& other) const
            {
                return Vector3d(x + other.x, y + other.y, z + other.z);
            }

            Vector3d operator-(const Vector3d& other) const
            {
                return Vector3d(x - other.x, y - other.y, z - other.z);
            }

            Vector3d operator*(const Vector3d& other) const
            {
                return Vector3d(x * other.x, y * other.y, z * other.z);
            }

            Vector3d operator/(const Vector3d& other) const
            {
                return Vector3d(x / other.x, y / other.y, z / other.z);
            }

            Vector3d operator+(double scalar) const
            {
                return Vector3d(x + scalar, y + scalar, z + scalar);
            }

            Vector3d operator-(double scalar) const
            {
                return Vector3d(x - scalar, y - scalar, z - scalar);
            }

            Vector3d operator*(double scalar) const
            {
                return Vector3d(x * scalar, y * scalar, z * scalar);
            }

            Vector3d operator/(double scalar) const
            {
                return Vector3d(x / scalar, y / scalar, z / scalar);
            }

            Vector3d& operator+=(const Vector3d& other)
            {
                x += other.x;
                y += other.y;
                z += other.z;

                return *this;
            }

            Vector3d& operator-=(const Vector3d& other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;

                return *this;
            }

            Vector3d& operator*=(const Vector3d& other)
            {
                x *= other.x;
                y *= other.y;
                z *= other.z;

                return *this;
            }

            Vector3d& operator/=(const Vector3d& other)
            {
                x /= other.x;
                y /= other.y;
                z /= other.z;

                return *this;
            }

            Vector3d& operator+=(double scalar)
            {
                x += scalar;
                y += scalar;
                z += scalar;

                return *this;
            }

            Vector3d& operator-=(double scalar)
            {
                x -= scalar;
                y -= scalar;
                z -= scalar;

                return *this;
            }

            Vector3d& operator*=(double scalar)
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;

                return *this;
            }

            Vector3d& operator/=(double scalar)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;

                return *this;
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, z, 0.0f);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3(x, y, z);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4(x, y, z, 0.0f);
            }

            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
        };

        bool operator==(const Vector3d& lhs, const Vector3d& rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        bool operator!=(const Vector3d& lhs, const Vector3d& rhs)
        {
            return !(lhs == rhs);
        }
    }
}