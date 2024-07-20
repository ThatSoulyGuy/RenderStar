#pragma once

#include <DirectXMath.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Math/Vector3.hpp"
#include "RenderStar/Util/Typedefs.hpp"

#define PI 3.141592653589793

using namespace RenderStar::Core;
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

            Quaternioni(Vector3i vector)
            {
                double pitch = static_cast<double>(vector.x) * PI / 180.0;
                double yaw = static_cast<double>(vector.y) * PI / 180.0; 
                double roll = static_cast<double>(vector.z) * PI / 180.0;

                double cr = cos(pitch * 0.5);
                double sr = sin(pitch * 0.5);
                double cp = cos(yaw * 0.5);
                double sp = sin(yaw * 0.5);
                double cy = cos(roll * 0.5);
                double sy = sin(roll * 0.5);

                w = static_cast<int>(cr * cp * cy + sr * sp * sy);
                x = static_cast<int>(sr * cp * cy - cr * sp * sy);
                y = static_cast<int>(cr * sp * cy + sr * cp * sy);
                z = static_cast<int>(cr * cp * sy - sr * sp * cy);

#ifndef SILENCE_QUATERNION
                Logger_WriteConsole("Deriving a Quaternioni from a Vector3i is not supported and is only included for compatability reasons. Expect precision issues. We recommend using the double-precision Quaterniond.\n#SILENCE_QUATERNION", LogLevel::WARNING);
#endif // SILENCE_QUATERNION
            }

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

            Quaternionf() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
            Quaternionf(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
            Quaternionf(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
            Quaternionf(const Quaternionf& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

            Quaternionf(DirectX::XMVECTOR vector)
            {
                DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(this), vector);
            }

            Quaternionf(Vector3f vector)
            {
                double pitch = static_cast<double>(vector.x) * PI / 180.0;
                double yaw = static_cast<double>(vector.y) * PI / 180.0;
                double roll = static_cast<double>(vector.z) * PI / 180.0;

                double cr = cos(pitch * 0.5);
                double sr = sin(pitch * 0.5);
                double cp = cos(yaw * 0.5);
                double sp = sin(yaw * 0.5);
                double cy = cos(roll * 0.5);
                double sy = sin(roll * 0.5);

                w = static_cast<float>(cr * cp * cy + sr * sp * sy);
                x = static_cast<float>(sr * cp * cy - cr * sp * sy);
                y = static_cast<float>(cr * sp * cy + sr * cp * sy);
                z = static_cast<float>(cr * cp * sy - sr * sp * cy);

#ifndef SILENCE_QUATERNION
                Logger_WriteConsole("Deriving a Quaternionf from a Vector3f is semi-supported and is only included for compatability reasons. Expect minimal precision issues. We recommend using the double-precision Quaterniond.\n#SILENCE_QUATERNION", LogLevel::WARNING);
#endif // !SILENCE_QUATERNION
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

            Quaterniond(Vector3d vector)
            {
                double pitch = static_cast<double>(vector.x) * PI / 180.0;
                double yaw = static_cast<double>(vector.y) * PI / 180.0;
                double roll = static_cast<double>(vector.z) * PI / 180.0;

                double cr = cos(pitch * 0.5);
                double sr = sin(pitch * 0.5);
                double cp = cos(yaw * 0.5);
                double sp = sin(yaw * 0.5);
                double cy = cos(roll * 0.5);
                double sy = sin(roll * 0.5);

                w = cr * cp * cy + sr * sp * sy;
                x = sr * cp * cy - cr * sp * sy;
                y = cr * sp * cy + sr * cp * sy;
                z = cr * cp * sy - sr * sp * cy;

#ifndef SILENCE_QUATERNION
                Logger_WriteConsole("While Quaterniond is the recommended for Quaternions, double-precision is not universially supported in DirectX 11, and by extension RenderStar. Expect compatability issues when integrating into DirectX.\n#SILENCE_QUATERNION", LogLevel::WARNING);
#endif // !SILENCE_QUATERNION
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