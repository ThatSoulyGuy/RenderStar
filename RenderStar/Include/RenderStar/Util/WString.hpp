#pragma once

#include "RenderStar/Util/Typedefs.hpp"
#include "RenderStar/Util/Vector.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace RenderStar
{
	namespace Util
	{
		class WString
		{
		public:

			WString() : buffer(L"") {}

			WString(const std::wstring& input) : buffer(input) {}

			WString(const wchar_t* input) : buffer(input) {}

			WString(const WString& other) : buffer(other.buffer) {}

			WString(WString&& other) noexcept : buffer(std::move(other.buffer)) {}

			WString(wchar_t input) : buffer(1, input) {}

			using const_iterator = std::wstring::const_iterator;

			WString& operator=(const WString& other)
			{
				buffer = other.buffer;
				return *this;
			}

			WString& operator=(WString&& other) noexcept
			{
				buffer = std::move(other.buffer);
				return *this;
			}

			WString& operator=(const std::wstring& input)
			{
				buffer = input;
				return *this;
			}

			WString& operator=(const wchar_t* input)
			{
				buffer = input;
				return *this;
			}

			WString& operator+=(const WString& other)
			{
				buffer += other.buffer;
				return *this;
			}

			WString& operator+=(const std::wstring& input)
			{
				buffer += input;
				return *this;
			}

			WString& operator+=(const wchar_t* input)
			{
				buffer += input;
				return *this;
			}

			WString& operator+=(wchar_t input)
			{
				buffer += input;
				return *this;
			}

			WString operator+(const WString& other) const
			{
				return WString(buffer + other.buffer);
			}

			WString operator+(const std::wstring& input) const
			{
				return WString(buffer + input);
			}

			WString operator+(const wchar_t* input) const
			{
				return WString(buffer + input);
			}

			WString operator+(wchar_t input) const
			{
				return WString(buffer + input);
			}

			bool operator==(const WString& other) const
			{
				return buffer == other.buffer;
			}

			bool operator==(const std::wstring& input) const
			{
				return buffer == input;
			}

			bool operator==(const wchar_t* input) const
			{
				return buffer == input;
			}

			bool operator!=(const WString& other) const
			{
				return buffer != other.buffer;
			}

			bool operator!=(const std::wstring& input) const
			{
				return buffer != input;
			}

			bool operator!=(const wchar_t* input) const
			{
				return buffer != input;
			}

			bool operator<(const WString& other) const
			{
				return buffer < other.buffer;
			}

			bool operator<(const std::wstring& input) const
			{
				return buffer < input;
			}

			bool operator<(const wchar_t* input) const
			{
				return buffer < input;
			}

			bool operator>(const WString& other) const
			{
				return buffer > other.buffer;
			}

			bool operator>(const std::wstring& input) const
			{
				return buffer > input;
			}

			bool operator>(const wchar_t* input) const
			{
				return buffer > input;
			}

			bool operator<=(const WString& other) const
			{
				return buffer <= other.buffer;
			}

			bool operator<=(const std::wstring& input) const
			{
				return buffer <= input;
			}

			bool operator<=(const wchar_t* input) const
			{
				return buffer <= input;
			}

			bool operator>=(const WString& other) const
			{
				return buffer >= other.buffer;
			}

			bool operator>=(const std::wstring& input) const
			{
				return buffer >= input;
			}

			bool operator>=(const wchar_t* input) const
			{
				return buffer >= input;
			}

			wchar_t operator[](Size index) const
			{
				return buffer[index];
			}

			wchar_t& operator[](Size index)
			{
				return buffer[index];
			}

			std::wstring& operator<<(const WString& input)
			{
				buffer += input.buffer;
				return buffer;
			}

			std::wstring& operator<<(const std::wstring& input)
			{
				buffer += input;
				return buffer;
			}

			std::wstring& operator<<(const wchar_t* input)
			{
				buffer += input;
				return buffer;
			}

			std::wstring& operator<<(wchar_t input)
			{
				buffer += input;
				return buffer;
			}

			std::wstring& operator>>(WString& input)
			{
				input = buffer;
				buffer.clear();
				return buffer;
			}

			std::wstring& operator>>(std::wstring& input)
			{
				input = buffer;
				buffer.clear();
				return buffer;
			}

			std::wstring& operator>>(wchar_t& input)
			{
				input = buffer[0];
				buffer.erase(0, 1);
				return buffer;
			}

			WOutputStream& operator<<(WOutputStream& stream) const
			{
				stream << buffer;
				return stream;
			}

			operator std::wstring() const
			{
				return buffer;
			}

			operator const wchar_t* () const
			{
				return buffer.c_str();
			}

			operator int32() const
			{
				return std::stoi(buffer);
			}

			operator float() const
			{
				return std::stof(buffer);
			}

			operator double() const
			{
				return std::stod(buffer);
			}

			operator bool() const
			{
				return buffer == L"true";
			}

			operator std::string() const
			{
				return std::string(buffer.begin(), buffer.end());
			}

			WString& operator-=(wchar_t input)
			{
				size_t pos = buffer.find_last_of(input);

				if (pos != std::wstring::npos)
					buffer.erase(pos, 1);

				return *this;
			}

			WString operator-(wchar_t input) const
			{
				WString temp = *this;

				temp -= input;

				return temp;
			}

			Size Length() const
			{
				return buffer.length();
			}

			void FindAndReplace(const std::wstring& find, const std::wstring& replace)
			{
				Size position = 0;

				while ((position = buffer.find(find, position)) != NotPosition)
				{
					buffer.replace(position, find.length(), replace);
					position += replace.length();
				}
			}

			void FindAndReplace(const wchar_t* find, const wchar_t* replace)
			{
				FindAndReplace(std::wstring(find), std::wstring(replace));
			}

			void FindAndReplace(const WString& find, const WString& replace)
			{
				FindAndReplace(find.buffer, replace.buffer);
			}

			void FindAndReplace(wchar_t find, wchar_t replace)
			{
				for (Size i = 0; i < buffer.length(); i++)
				{
					if (buffer[i] == find)
						buffer[i] = replace;
				}
			}

			Size Find(const std::wstring& input) const
			{
				return buffer.find(input);
			}

			Size Find(const wchar_t* input) const
			{
				return buffer.find(input);
			}

			Size Find(const WString& input) const
			{
				return buffer.find(input.buffer);
			}

			Size Find(wchar_t input) const
			{
				return buffer.find(input);
			}

			Size FindLast(const std::wstring& input) const
			{
				return buffer.rfind(input);
			}

			Size FindLast(const wchar_t* input) const
			{
				return buffer.rfind(input);
			}

			Size FindLast(const WString& input) const
			{
				return buffer.rfind(input.buffer);
			}

			Size FindLast(wchar_t input) const
			{
				return buffer.rfind(input);
			}

			void Remove(Size position, Size length)
			{
				buffer.erase(position, length);
			}

			void Remove(wchar_t input)
			{
				Size position = 0;

				while ((position = buffer.find(input, position)) != NotPosition)
				{
					buffer.erase(position, 1);
				}
			}

			bool IsEmpty() const
			{
				return buffer.empty();
			}

			void Remove(const std::wstring& input)
			{
				Size position = 0;

				while ((position = buffer.find(input, position)) != NotPosition)
				{
					buffer.erase(position, input.length());
				}
			}

			void Remove(const wchar_t* input)
			{
				Remove(std::wstring(input));
			}

			void Remove(const WString& input)
			{
				Remove(input.buffer);
			}

			void Remove(Size position)
			{
				buffer.erase(position, 1);
			}

			void RemoveLast(Size length)
			{
				buffer.erase(buffer.length() - length, length);
			}

			void RemoveLast(wchar_t input)
			{
				Size position = buffer.rfind(input);

				if (position != NotPosition)
					buffer.erase(position, 1);
			}

			void RemoveLast(const std::wstring& input)
			{
				Size position = buffer.rfind(input);

				if (position != NotPosition)
					buffer.erase(position, input.length());
			}

			void RemoveLast(const wchar_t* input)
			{
				RemoveLast(std::wstring(input));
			}

			void RemoveLast(const WString& input)
			{
				RemoveLast(input.buffer);
			}

			void RemoveFirst(Size length)
			{
				buffer.erase(0, length);
			}

			void RemoveFirst(wchar_t input)
			{
				Size position = buffer.find(input);

				if (position != NotPosition)
					buffer.erase(position, 1);
			}

			void RemoveFirst(const std::wstring& input)
			{
				Size position = buffer.find(input);

				if (position != NotPosition)
					buffer.erase(position, input.length());
			}

			void RemoveFirst(const wchar_t* input)
			{
				RemoveFirst(std::wstring(input));
			}

			void RemoveFirst(const WString& input)
			{
				RemoveFirst(input.buffer);
			}

			void Resize(Size size)
			{
				buffer.resize(size);
			}

			void SubWString(Size position, Size length)
			{
				buffer = buffer.substr(position, length);
			}

			void SubWString(Size position)
			{
				buffer = buffer.substr(position);
			}

			WString SubWString(Size position, Size length) const
			{
				return buffer.substr(position, length);
			}

			WString SubWString(Size position) const
			{
				return buffer.substr(position);
			}

			bool Contains(const std::wstring& input) const
			{
				return buffer.find(input) != NotPosition;
			}

			bool Contains(const wchar_t* input) const
			{
				return buffer.find(input) != NotPosition;
			}

			bool Contains(const WString& input) const
			{
				return buffer.find(input.buffer) != NotPosition;
			}

			bool StartsWith(const std::wstring& input) const
			{
				return buffer.find(input) == 0;
			}

			bool StartsWith(const wchar_t* input) const
			{
				return buffer.find(input) == 0;
			}

			bool StartsWith(const WString& input) const
			{
				return buffer.find(input.buffer) == 0;
			}

			bool EndsWith(const std::wstring& input) const
			{
				return buffer.rfind(input) == buffer.length() - input.length();
			}

			bool EndsWith(const wchar_t* input) const
			{
				return buffer.rfind(input) == buffer.length() - uaw_lstrlenW(input);
			}

			bool EndsWith(const WString& input) const
			{
				return buffer.rfind(input.buffer) == buffer.length() - input.buffer.length();
			}

			Vector<WString> Split(const std::wstring& delimiter) const
			{
				Vector<WString> result;

				Size position = 0;
				Size nextPosition = 0;

				while ((nextPosition = buffer.find(delimiter, position)) != NotPosition)
				{
					result += (buffer.substr(position, nextPosition - position));
					position = nextPosition + delimiter.length();
				}

				result += (buffer.substr(position, buffer.length() - position));

				return result;
			}

			Vector<WString> Split(const wchar_t* delimiter) const
			{
				return Split(std::wstring(delimiter));
			}

			Vector<WString> Split(const WString& delimiter) const
			{
				return Split(delimiter.buffer);
			}

			void ToLower()
			{
				std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);
			}

			void ToUpper()
			{
				std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::toupper);
			}

			void Trim()
			{
				buffer.erase(0, buffer.find_first_not_of(L" \t\n\r\f\v"));
				buffer.erase(buffer.find_last_not_of(L" \t\n\r\f\v") + 1);
			}

			void TrimLeft()
			{
				buffer.erase(0, buffer.find_first_not_of(L" \t\n\r\f\v"));
			}

			void TrimRight()
			{
				buffer.erase(buffer.find_last_not_of(L" \t\n\r\f\v") + 1);
			}

			auto begin()
			{
				return buffer.begin();
			}

			auto end()
			{
				return buffer.end();
			}

			void Clear()
			{
				buffer.clear();
			}

			static WString FromInt(int32 input)
			{
				return std::to_wstring(input);
			}

			static WString FromFloat(float input)
			{
				return std::to_wstring(input);
			}

			static WString FromDouble(double input)
			{
				return std::to_wstring(input);
			}

			static WString FromBool(bool input)
			{
				return input ? L"true" : L"false";
			}

			static WString FromChar(wchar_t input)
			{
				return std::wstring(1, input);
			}

			static WString FromVector(const Vector<WString>& input, const std::wstring& delimiter)
			{
				WString result;

				for (Size i = 0; i < input.Length(); i++)
				{
					result += input[i];

					if (i != input.Length() - 1)
						result += delimiter;
				}

				return result;
			}

			static WString FromVector(const Vector<WString>& input, const wchar_t* delimiter)
			{
				return FromVector(input, std::wstring(delimiter));
			}

			static WString FromVector(const Vector<WString>& input, const WString& delimiter)
			{
				return FromVector(input, delimiter.buffer);
			}

			static WString FromVector(const std::vector<WString>& input, const std::wstring& delimiter)
			{
				WString result;

				for (Size i = 0; i < input.size(); i++)
				{
					result += input[i];

					if (i != input.size() - 1)
						result += delimiter;
				}

				return result;
			}

			static WString FromVector(const std::vector<WString>& input, const wchar_t* delimiter)
			{
				return FromVector(input, std::wstring(delimiter));
			}

			const static Size NotPosition;

		private:

			std::wstring buffer;

		};

		std::ostream& operator<<(std::ostream& os, const WString& str)
		{
			std::wstring raw = str;
			os << raw;
			return os;
		}

		WString operator+(wchar_t lhs, const WString& rhs)
		{
			return WString(lhs) + rhs;
		}

		WString operator+(const std::wstring& lhs, const WString& rhs)
		{
			return WString(lhs) + rhs;
		}

		WString operator+(const wchar_t* lhs, const WString& rhs)
		{
			return WString(lhs) + rhs;
		}

		bool operator==(const std::wstring& lhs, const WString& rhs)
		{
			std::wstring raw = rhs;
			return lhs == raw;
		}

		bool operator==(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs == raw;
		}

		bool operator!=(const std::wstring& lhs, const WString& rhs)
		{
			std::wstring raw = rhs;
			return lhs != raw;
		}

		bool operator!=(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs != raw;
		}

		bool operator<(const std::wstring& lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs < raw;
		}

		bool operator<(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs < raw;
		}

		bool operator>(const std::wstring& lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs > raw;
		}

		bool operator>(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs > raw;
		}

		bool operator<=(const std::wstring& lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs <= raw;
		}

		bool operator<=(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs <= raw;
		}

		bool operator>=(const std::wstring& lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs >= raw;
		}

		bool operator>=(const wchar_t* lhs, const WString& rhs)
		{
			const wchar_t* raw = rhs;
			return lhs >= raw;
		}

		WString& operator<<(WString& lhs, const WString& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		WString& operator<<(WString& lhs, const std::wstring& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		WString& operator<<(WString& lhs, const wchar_t* rhs)
		{
			lhs += rhs;
			return lhs;
		}

		WString& operator<<(WString& lhs, wchar_t rhs)
		{
			lhs += rhs;
			return lhs;
		}

		WString& operator>>(WString& lhs, WString& rhs)
		{
			rhs = lhs;
			lhs.Clear();
			return lhs;
		}

		WString& operator>>(WString& lhs, wchar_t& rhs)
		{
			rhs = lhs[0];
			lhs.Remove((Size)0);
			return lhs;
		}

		const Size WString::NotPosition = std::wstring::npos;
	}
}

/*
namespace std
{
	template <>
	struct formatter<RenderStar::Util::WString> : formatter<std::wstring>
	{
		auto format(const RenderStar::Util::WString& str, format_context& ctx) const
		{
			std::wstring message = str;
			return formatter<std::wstring>::format(message, ctx);
		}
	};
}*/