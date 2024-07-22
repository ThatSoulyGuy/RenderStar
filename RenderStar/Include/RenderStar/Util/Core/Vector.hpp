#pragma once

#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
	namespace Util
	{
		template<typename T>
		class Vector
		{
		public:

			Vector()
			{
				data = std::vector<T>();
			}

			Vector(const std::vector<T>& input)
			{
				data = input;
			}

			Vector(const Vector& other)
			{
				data = other.data;
			}

			Vector(Vector&& other) noexcept
			{
				data = std::move(other.data);
			}

			Vector(std::initializer_list<T> list)
			{
				data = list;
			}

			Vector& operator=(const Vector& other)
			{
				data = other.data;

				return *this;
			}

			Vector& operator=(Vector&& other)
			{
				data = std::move(other.data);

				return *this;
			}

			Vector& operator=(const std::vector<T>& input)
			{
				data = input;

				return *this;
			}

			Vector& operator=(std::vector<T>&& input)
			{
				data = std::move(input);

				return *this;
			}

			Vector& operator+=(const Vector& other)
			{
				data.insert(data.end(), other.data.begin(), other.data.end());

				return *this;
			}

			Vector& operator+=(const T& value)
			{
				data.push_back(value);

				return *this;
			}

			Vector& operator+=(T&& value)
			{
				data.push_back(std::move(value));

				return *this;
			}

			Vector operator+(const Vector& other) const
			{
				Vector result = *this;

				result += other;

				return result;
			}

			Vector operator+(const T& value) const
			{
				Vector result = *this;

				result += value;

				return result;
			}

			Vector operator+(T&& value) const
			{
				Vector result = *this;

				result += std::move(value);

				return result;
			}

			Vector operator-(const Vector& other) const
			{
				Vector result = *this;

				for (const T& value : other.data)
				{
					auto it = std::find(result.data.begin(), result.data.end(), value);

					if (it != result.data.end())
						result.data.erase(it);
				}

				return result;
			}

			Vector operator-(const T& value) const
			{
				Vector result = *this;

				auto it = std::find(result.data.begin(), result.data.end(), value);

				if (it != result.data.end())
					result.data.erase(it);
				
				return result;
			}

			Vector operator-(T&& value) const
			{
				Vector result = *this;

				auto it = std::find(result.data.begin(), result.data.end(), value);

				if (it != result.data.end())
					result.data.erase(it);
				
				return result;
			}

			Vector& operator-=(const Vector& other)
			{
				for (const T& value : other.data)
				{
					auto it = std::find(data.begin(), data.end(), value);

					if (it != data.end())
						data.erase(it);
				}

				return *this;
			}

			Vector& operator-=(const T& value)
			{
				auto it = std::find(data.begin(), data.end(), value);

				if (it != data.end())
					data.erase(it);
				
				return *this;
			}

			Vector& operator-=(T&& value)
			{
				auto it = std::find(data.begin(), data.end(), value);
				if (it != data.end())
				{
					data.erase(it);
				}
				return *this;
			}

			bool operator==(const std::vector<T>& input) const
			{
				return data == input;
			}

			bool operator==(const Vector& other) const
			{
				return data == other.data;
			}

			bool operator!=(const Vector& other) const
			{
				return data != other.data;
			}

			operator std::vector<T>() const
			{
				return data;
			}

			operator const void*()
			{
				return data.data();
			}

			T& Front()
			{
				return data.front();
			}

			T& At(Size index)
			{
				return data.at(index);
			}

			const T& At(Size index) const
			{
				return data.at(index);
			}

			T& operator[](Size index)
			{
				return data[index];
			}

			const T& operator[](Size index) const
			{
				return data[index];
			}

			Size Length() const
			{
				return data.size();
			}

			void Clear()
			{
				data.clear();
			}

			void Reserve(Size size)
			{
				data.reserve(size);
			}

			void Resize(Size size)
			{
				data.resize(size);
			}

			void Resize(Size size, const T& value)
			{
				data.resize(size, value);
			}

			void Resize(Size size, T&& value)
			{
				data.resize(size, std::move(value));
			}

			void ShrinkToFit()
			{
				data.shrink_to_fit();
			}

			auto begin()
			{
				return data.begin();
			}

			auto end()
			{
				return data.end();
			}

			void Remove(Size index)
			{
				data.erase(data.begin() + index);
			}

			void Remove(Size start, Size end)
			{
				data.erase(data.begin() + start, data.begin() + end);
			}

		private:

			std::vector<T> data;

		};
	}
}