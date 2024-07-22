#pragma once

#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
	namespace Util
	{
		namespace Core
		{
			template<typename T, Size A>
			class Array
			{

			public:

				Array()
				{
					data = std::array<T, A>();
				}

				Array(const std::array<T, A>& input)
				{
					data = input;
				}

				Array(const Array& other)
				{
					data = other.data;
				}

				Array(Array&& other)
				{
					data = std::move(other.data);
				}

				Array(std::initializer_list<T> list)
				{
					if (list.size() != A)
						throw std::invalid_argument("Initializer list size does not match array size.");

					std::copy(list.begin(), list.end(), data.begin());
				}

				Array& operator=(const Array& other)
				{
					data = other.data;

					return *this;
				}

				Array& operator=(Array&& other)
				{
					data = std::move(other.data);

					return *this;
				}

				Array& operator=(const std::array<T, A>& input)
				{
					data = input;

					return *this;
				}

				Array& operator=(std::array<T, A>&& input)
				{
					data = std::move(input);

					return *this;
				}

				T& operator[](Size index) const
				{
					return data[index];
				}

				bool operator==(const Array<T, A>& other) const
				{
					for (Size i = 0; i < A; i++)
					{
						if (data[i] != other.data[i])
							return false;
					}

					return true;
				}

				bool operator==(const std::array<T, A>& other) const
				{
					for (Size i = 0; i < A; i++)
					{
						if (data[i] != other[i])
							return false;
					}

					return true;
				}

				bool operator==(const Array&& other)
				{
					Array temp = std::move(other);

					for (Size i = 0; i < A; i++)
					{
						if (data[i] != temp.data[i])
							return false;
					}

					return true;
				}

				operator std::array<T, A>() const
				{
					return data;
				}

				operator const T*() const
				{
					return data.data();
				}

				Size Length() const
				{
					return A;
				}

			private:

				std::array<T, A> data;

			};
		}
	}
}