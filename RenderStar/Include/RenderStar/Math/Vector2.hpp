#pragma once

#include <DirectXMath.h>
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
    namespace Math
    {
        class Vector2i
        {

        public:

            Vector2i() : x(0), y(0) {}
            Vector2i(int x, int y) : x(x), y(y) {}
            Vector2i(int scalar) : x(scalar), y(scalar) {}
            Vector2i(const Vector2i& other) : x(other.x), y(other.y) {}

            Vector2i(const DirectX::XMVECTOR& vector)
            {
                x = (int)DirectX::XMVectorGetX(vector);
                y = (int)DirectX::XMVectorGetY(vector);
            }

            Vector2i& operator=(const Vector2i& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                }

                return *this;
            }

            Vector2i& operator=(const DirectX::XMVECTOR& vector)
            {
                x = (int)DirectX::XMVectorGetX(vector);
                y = (int)DirectX::XMVectorGetY(vector);

                return *this;
            }

            Vector2i operator+(const Vector2i& other) const
            {
                return Vector2i(x + other.x, y + other.y);
            }

            Vector2i operator-(const Vector2i& other) const
            {
                return Vector2i(x - other.x, y - other.y);
            }

            Vector2i operator*(const Vector2i& other) const
            {
                return Vector2i(x * other.x, y * other.y);
            }

            Vector2i operator/(const Vector2i& other) const
            {
                return Vector2i(x / other.x, y / other.y);
            }

            Vector2i operator+(int scalar) const
            {
                return Vector2i(x + scalar, y + scalar);
            }

            Vector2i operator-(int scalar) const
            {
                return Vector2i(x - scalar, y - scalar);
            }

            Vector2i operator*(int scalar) const
            {
                return Vector2i(x * scalar, y * scalar);
            }

            Vector2i operator/(int scalar) const
            {
                return Vector2i(x / scalar, y / scalar);
            }

            Vector2i& operator+=(const Vector2i& other)
            {
                x += other.x;
                y += other.y;

                return *this;
            }

            Vector2i& operator-=(const Vector2i& other)
            {
                x -= other.x;
                y -= other.y;

                return *this;
            }

            Vector2i& operator*=(const Vector2i& other)
            {
                x *= other.x;
                y *= other.y;

                return *this;
            }

            Vector2i& operator/=(const Vector2i& other)
            {
                x /= other.x;
                y /= other.y;

                return *this;
            }

            Vector2i& operator+=(int scalar)
            {
                x += scalar;
                y += scalar;

                return *this;
            }

            Vector2i& operator-=(int scalar)
            {
                x -= scalar;
                y -= scalar;

                return *this;
            }

            Vector2i& operator*=(int scalar)
            {
                x *= scalar;
                y *= scalar;

                return *this;
            }

            Vector2i& operator/=(int scalar)
            {
                x /= scalar;
                y /= scalar;

                return *this;
            }

            operator DirectX::XMINT2() const
            {
                return DirectX::XMINT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet((float)x, (float)y, 0, 0);
            }

            operator DirectX::XMINT3() const
            {
                return DirectX::XMINT3((int)x, (int)y, 0);
            }

            operator DirectX::XMINT4() const
            {
                return DirectX::XMINT4((int)x, (int)y, 0, 0);
            }

            int x = 0;
            int y = 0;
        };

        class Vector2f
        {

        public:

            Vector2f() : x(0.0f), y(0.0f) {}
            Vector2f(float x, float y) : x(x), y(y) {}
            Vector2f(float scalar) : x(scalar), y(scalar) {}
            Vector2f(const Vector2f& other) : x(other.x), y(other.y) {}

            Vector2f(const DirectX::XMVECTOR& vector)
            {
                x = DirectX::XMVectorGetX(vector);
                y = DirectX::XMVectorGetY(vector);
            }

            Vector2f& operator=(const Vector2f& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                }

                return *this;
            }

            Vector2f& operator=(const DirectX::XMVECTOR& vector)
            {
                x = DirectX::XMVectorGetX(vector);
                y = DirectX::XMVectorGetY(vector);

                return *this;
            }

            Vector2f operator+(const Vector2f& other) const
            {
                return Vector2f(x + other.x, y + other.y);
            }

            Vector2f operator-(const Vector2f& other) const
            {
                return Vector2f(x - other.x, y - other.y);
            }

            Vector2f operator*(const Vector2f& other) const
            {
                return Vector2f(x * other.x, y * other.y);
            }

            Vector2f operator/(const Vector2f& other) const
            {
                return Vector2f(x / other.x, y / other.y);
            }

            Vector2f operator+(float scalar) const
            {
                return Vector2f(x + scalar, y + scalar);
            }

            Vector2f operator-(float scalar) const
            {
                return Vector2f(x - scalar, y - scalar);
            }

            Vector2f operator*(float scalar) const
            {
                return Vector2f(x * scalar, y * scalar);
            }

            Vector2f operator/(float scalar) const
            {
                return Vector2f(x / scalar, y / scalar);
            }

            Vector2f& operator+=(const Vector2f& other)
            {
                x += other.x;
                y += other.y;

                return *this;
            }

            Vector2f& operator-=(const Vector2f& other)
            {
                x -= other.x;
                y -= other.y;

                return *this;
            }

            Vector2f& operator*=(const Vector2f& other)
            {
                x *= other.x;
                y *= other.y;

                return *this;
            }

            Vector2f& operator/=(const Vector2f& other)
            {
                x /= other.x;
                y /= other.y;

                return *this;
            }

            Vector2f& operator+=(float scalar)
            {
                x += scalar;
                y += scalar;

                return *this;
            }

            Vector2f& operator-=(float scalar)
            {
                x -= scalar;
                y -= scalar;

                return *this;
            }

            Vector2f& operator*=(float scalar)
            {
                x *= scalar;
                y *= scalar;

                return *this;
            }

            Vector2f& operator/=(float scalar)
            {
                x /= scalar;
                y /= scalar;

                return *this;
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2(x, y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3(x, y, 0.0f);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4(x, y, 0.0f, 0.0f);
            }

            float x = 0.0f;
            float y = 0.0f;
        };

        class Vector2d
        {

        public:

            Vector2d() : x(0.0f), y(0.0f) {}
            Vector2d(double x, double y) : x(x), y(y) {}
            Vector2d(double scalar) : x(scalar), y(scalar) {}
            Vector2d(const Vector2d& other) : x(other.x), y(other.y) {}

            Vector2d(const DirectX::XMVECTOR& vector)
            {
                x = DirectX::XMVectorGetX(vector);
                y = DirectX::XMVectorGetY(vector);
            }

            Vector2d& operator=(const Vector2d& other)
            {
                if (this != &other)
                {
                    x = other.x;
                    y = other.y;
                }

                return *this;
            }

            Vector2d& operator=(const DirectX::XMVECTOR& vector)
            {
                x = DirectX::XMVectorGetX(vector);
                y = DirectX::XMVectorGetY(vector);

                return *this;
            }

            Vector2d operator+(const Vector2d& other) const
            {
                return Vector2d(x + other.x, y + other.y);
            }

            Vector2d operator-(const Vector2d& other) const
            {
                return Vector2d(x - other.x, y - other.y);
            }

            Vector2d operator*(const Vector2d& other) const
            {
                return Vector2d(x * other.x, y * other.y);
            }

            Vector2d operator/(const Vector2d& other) const
            {
                return Vector2d(x / other.x, y / other.y);
            }

            Vector2d operator+(double scalar) const
            {
                return Vector2d(x + scalar, y + scalar);
            }

            Vector2d operator-(double scalar) const
            {
                return Vector2d(x - scalar, y - scalar);
            }

            Vector2d operator*(double scalar) const
            {
                return Vector2d(x * scalar, y * scalar);
            }

            Vector2d operator/(double scalar) const
            {
                return Vector2d(x / scalar, y / scalar);
            }

            Vector2d& operator+=(const Vector2d& other)
            {
                x += other.x;
                y += other.y;

                return *this;
            }

            Vector2d& operator-=(const Vector2d& other)
            {
                x -= other.x;
                y -= other.y;

                return *this;
            }

            Vector2d& operator*=(const Vector2d& other)
            {
                x *= other.x;
                y *= other.y;

                return *this;
            }

            Vector2d& operator/=(const Vector2d& other)
            {
                x /= other.x;
                y /= other.y;

                return *this;
            }

            Vector2d& operator+=(double scalar)
            {
                x += scalar;
                y += scalar;

                return *this;
            }

            Vector2d& operator-=(double scalar)
            {
                x -= scalar;
                y -= scalar;

                return *this;
            }

            Vector2d& operator*=(double scalar)
            {
                x *= scalar;
                y *= scalar;

                return *this;
            }

            Vector2d& operator/=(double scalar)
            {
                x /= scalar;
                y /= scalar;

                return *this;
            }

            operator DirectX::XMFLOAT2() const
            {
                return DirectX::XMFLOAT2((double)x, (double)y);
            }

            operator DirectX::XMVECTOR() const
            {
                return DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
            }

            operator DirectX::XMFLOAT3() const
            {
                return DirectX::XMFLOAT3((double)x, (double)y, 0.0f);
            }

            operator DirectX::XMFLOAT4() const
            {
                return DirectX::XMFLOAT4((double)x, (double)y, 0.0f, 0.0f);
            }

            double x = 0.0;
            double y = 0.0;
        };
    }
}