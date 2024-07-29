#pragma once

#include "RenderStar/Util/Typedefs.hpp"
#include "RenderStar/Util/Core/Vector.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace RenderStar
{
	namespace Util
	{
		namespace Core
		{
			class String
			{
			public:

				String() : buffer("") {}

				String(const std::string& input) : buffer(input) {}

				String(const char* input) : buffer(input) {}

				String(const String& other) : buffer(other.buffer) {}

				String(String&& other) noexcept : buffer(std::move(other.buffer)) {}

				String(char input) : buffer(1, input) {}

				using const_iterator = std::string::const_iterator;

				String& operator=(const String& other)
				{
					buffer = other.buffer;
					return *this;
				}

				String& operator=(String&& other) noexcept
				{
					buffer = std::move(other.buffer);
					return *this;
				}

				String& operator=(const std::string& input)
				{
					buffer = input;
					return *this;
				}

				String& operator=(const char* input)
				{
					buffer = input;
					return *this;
				}

				String& operator+=(const String& other)
				{
					buffer += other.buffer;
					return *this;
				}

				String& operator+=(const std::string& input)
				{
					buffer += input;
					return *this;
				}

				String& operator+=(const char* input)
				{
					buffer += input;
					return *this;
				}

				String& operator+=(char input)
				{
					buffer += input;
					return *this;
				}

				String operator+(const String& other) const
				{
					return String(buffer + other.buffer);
				}

				String operator+(const std::string& input) const
				{
					return String(buffer + input);
				}

				String operator+(const char* input) const
				{
					return String(buffer + input);
				}

				String operator+(char input) const
				{
					return String(buffer + input);
				}

				bool operator==(const String& other) const
				{
					return buffer == other.buffer;
				}

				bool operator==(const std::string& input) const
				{
					return buffer == input;
				}

				bool operator==(const char* input) const
				{
					return buffer == input;
				}

				bool operator!=(const String& other) const
				{
					return buffer != other.buffer;
				}

				bool operator!=(const std::string& input) const
				{
					return buffer != input;
				}

				bool operator!=(const char* input) const
				{
					return buffer != input;
				}

				bool operator<(const String& other) const
				{
					return buffer < other.buffer;
				}

				bool operator<(const std::string& input) const
				{
					return buffer < input;
				}

				bool operator<(const char* input) const
				{
					return buffer < input;
				}

				bool operator>(const String& other) const
				{
					return buffer > other.buffer;
				}

				bool operator>(const std::string& input) const
				{
					return buffer > input;
				}

				bool operator>(const char* input) const
				{
					return buffer > input;
				}

				bool operator<=(const String& other) const
				{
					return buffer <= other.buffer;
				}

				bool operator<=(const std::string& input) const
				{
					return buffer <= input;
				}

				bool operator<=(const char* input) const
				{
					return buffer <= input;
				}

				bool operator>=(const String& other) const
				{
					return buffer >= other.buffer;
				}

				bool operator>=(const std::string& input) const
				{
					return buffer >= input;
				}

				bool operator>=(const char* input) const
				{
					return buffer >= input;
				}

				char operator[](Size index) const
				{
					return buffer[index];
				}

				char& operator[](Size index)
				{
					return buffer[index];
				}

				std::string& operator<<(const String& input)
				{
					buffer += input.buffer;
					return buffer;
				}

				std::string& operator<<(const std::string& input)
				{
					buffer += input;
					return buffer;
				}

				std::string& operator<<(const char* input)
				{
					buffer += input;
					return buffer;
				}

				std::string& operator<<(char input)
				{
					buffer += input;
					return buffer;
				}

				std::string& operator>>(String& input)
				{
					input = buffer;
					buffer.clear();
					return buffer;
				}

				std::string& operator>>(std::string& input)
				{
					input = buffer;
					buffer.clear();
					return buffer;
				}

				std::string& operator>>(char* input)
				{
					strcpy_s(input, buffer.length() + 1, buffer.c_str());
					buffer.clear();
					return buffer;
				}

				std::string& operator>>(char& input)
				{
					input = buffer[0];
					buffer.erase(0, 1);
					return buffer;
				}

				OutputStream& operator<<(OutputStream& stream) const
				{
					stream << buffer;
					return stream;
				}

				operator std::string() const
				{
					return buffer;
				}

				operator const char* () const
				{
					return buffer.c_str();
				}

				operator std::wstring() const
				{
					return std::wstring(buffer.begin(), buffer.end());
				}

				mutable std::wstring string;
				operator const wchar_t* () const
				{
					string = std::wstring(buffer.begin(), buffer.end());

					return string.c_str();
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
					return buffer == "true";
				}

				String& operator-=(char input)
				{
					size_t pos = buffer.find_last_of(input);

					if (pos != std::string::npos)
						buffer.erase(pos, 1);

					return *this;
				}

				String operator-(char input) const
				{
					String temp = *this;

					temp -= input;

					return temp;
				}

				Size Length() const
				{
					return buffer.length();
				}

				void FindAndReplace(const std::string& find, const std::string& replace)
				{
					Size position = 0;

					while ((position = buffer.find(find, position)) != NotPosition)
					{
						buffer.replace(position, find.length(), replace);
						position += replace.length();
					}
				}

				void FindAndReplace(const char* find, const char* replace)
				{
					FindAndReplace(std::string(find), std::string(replace));
				}

				void FindAndReplace(const String& find, const String& replace)
				{
					FindAndReplace(find.buffer, replace.buffer);
				}

				void FindAndReplace(char find, char replace)
				{
					for (Size i = 0; i < buffer.length(); i++)
					{
						if (buffer[i] == find)
							buffer[i] = replace;
					}
				}

				Size Find(const std::string& input) const
				{
					return buffer.find(input);
				}

				Size Find(const char* input) const
				{
					return buffer.find(input);
				}

				Size Find(const String& input) const
				{
					return buffer.find(input.buffer);
				}

				Size Find(char input) const
				{
					return buffer.find(input);
				}

				Size FindLast(const std::string& input) const
				{
					return buffer.rfind(input);
				}

				Size FindLast(const char* input) const
				{
					return buffer.rfind(input);
				}

				Size FindLast(const String& input) const
				{
					return buffer.rfind(input.buffer);
				}

				Size FindLast(char input) const
				{
					return buffer.rfind(input);
				}

				void Remove(Size position, Size length)
				{
					buffer.erase(position, length);
				}

				void Remove(char input)
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

				void Remove(const std::string& input)
				{
					Size position = 0;

					while ((position = buffer.find(input, position)) != NotPosition)
					{
						buffer.erase(position, input.length());
					}
				}

				void Remove(const char* input)
				{
					Remove(std::string(input));
				}

				void Remove(const String& input)
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

				void RemoveLast(char input)
				{
					Size position = buffer.rfind(input);

					if (position != NotPosition)
						buffer.erase(position, 1);
				}

				void RemoveLast(const std::string& input)
				{
					Size position = buffer.rfind(input);

					if (position != NotPosition)
						buffer.erase(position, input.length());
				}

				void RemoveLast(const char* input)
				{
					RemoveLast(std::string(input));
				}

				void RemoveLast(const String& input)
				{
					RemoveLast(input.buffer);
				}

				void RemoveFirst(Size length)
				{
					buffer.erase(0, length);
				}

				void RemoveFirst(char input)
				{
					Size position = buffer.find(input);

					if (position != NotPosition)
						buffer.erase(position, 1);
				}

				void RemoveFirst(const std::string& input)
				{
					Size position = buffer.find(input);

					if (position != NotPosition)
						buffer.erase(position, input.length());
				}

				void RemoveFirst(const char* input)
				{
					RemoveFirst(std::string(input));
				}

				void RemoveFirst(const String& input)
				{
					RemoveFirst(input.buffer);
				}

				void Resize(Size size)
				{
					buffer.resize(size);
				}

				void SubString(Size position, Size length)
				{
					buffer = buffer.substr(position, length);
				}

				void SubString(Size position)
				{
					buffer = buffer.substr(position);
				}

				String SubString(Size position, Size length) const
				{
					return buffer.substr(position, length);
				}

				String SubString(Size position) const
				{
					return buffer.substr(position);
				}

				bool Contains(const std::string& input) const
				{
					return buffer.find(input) != NotPosition;
				}

				bool Contains(const char* input) const
				{
					return buffer.find(input) != NotPosition;
				}

				bool Contains(const String& input) const
				{
					return buffer.find(input.buffer) != NotPosition;
				}

				bool StartsWith(const std::string& input) const
				{
					return buffer.find(input) == 0;
				}

				bool StartsWith(const char* input) const
				{
					return buffer.find(input) == 0;
				}

				bool StartsWith(const String& input) const
				{
					return buffer.find(input.buffer) == 0;
				}

				bool EndsWith(const std::string& input) const
				{
					return buffer.rfind(input) == buffer.length() - input.length();
				}

				bool EndsWith(const char* input) const
				{
					return buffer.rfind(input) == buffer.length() - strlen(input);
				}

				bool EndsWith(const String& input) const
				{
					return buffer.rfind(input.buffer) == buffer.length() - input.buffer.length();
				}

				Vector<String> Split(const std::string& delimiter) const
				{
					Vector<String> result;

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

				Vector<String> Split(const char* delimiter) const
				{
					return Split(std::string(delimiter));
				}

				Vector<String> Split(const String& delimiter) const
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
					buffer.erase(0, buffer.find_first_not_of(" \t\n\r\f\v"));
					buffer.erase(buffer.find_last_not_of(" \t\n\r\f\v") + 1);
				}

				void TrimLeft()
				{
					buffer.erase(0, buffer.find_first_not_of(" \t\n\r\f\v"));
				}

				void TrimRight()
				{
					buffer.erase(buffer.find_last_not_of(" \t\n\r\f\v") + 1);
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

				static String FromInt(int32 input)
				{
					return std::to_string(input);
				}

				static String FromFloat(float input)
				{
					return std::to_string(input);
				}

				static String FromDouble(double input)
				{
					return std::to_string(input);
				}

				static String FromBool(bool input)
				{
					return input ? "true" : "false";
				}

				static String FromChar(char input)
				{
					return std::string(1, input);
				}

				static String FromVector(const Vector<String>& input, const std::string& delimiter)
				{
					String result;

					for (Size i = 0; i < input.Length(); i++)
					{
						result += input[i];

						if (i != input.Length() - 1)
							result += delimiter;
					}

					return result;
				}

				static String FromVector(const Vector<String>& input, const char* delimiter)
				{
					return FromVector(input, std::string(delimiter));
				}

				static String FromVector(const Vector<String>& input, const String& delimiter)
				{
					return FromVector(input, delimiter.buffer);
				}

				static String FromVector(const std::vector<String>& input, const std::string& delimiter)
				{
					String result;

					for (Size i = 0; i < input.size(); i++)
					{
						result += input[i];

						if (i != input.size() - 1)
							result += delimiter;
					}

					return result;
				}

				static String FromVector(const std::vector<String>& input, const char* delimiter)
				{
					return FromVector(input, std::string(delimiter));
				}

				const static Size NotPosition;

			private:

				std::string buffer;

			};
			
			std::ostream& operator<<(std::ostream& os, const String& str)
			{
				return os << str.operator std::string();
			}

			String operator+(char lhs, const String& rhs)
			{
				return String(lhs) + rhs;
			}

			String operator+(const std::string& lhs, const String& rhs)
			{
				return String(lhs) + rhs;
			}

			String operator+(const char* lhs, const String& rhs)
			{
				return String(lhs) + rhs;
			}

			bool operator==(const std::string& lhs, const String& rhs)
			{
				return lhs == rhs.operator std::string();
			}

			bool operator==(const char* lhs, const String& rhs)
			{
				return lhs == rhs.operator const char*();
			}

			bool operator!=(const std::string& lhs, const String& rhs)
			{
				return lhs != rhs.operator std::string();
			}

			bool operator!=(const char* lhs, const String& rhs)
			{
				return lhs != rhs.operator const char*();
			}

			bool operator<(const std::string& lhs, const String& rhs)
			{
				return lhs < rhs.operator const char*();
			}

			bool operator<(const char* lhs, const String& rhs)
			{
				return lhs < rhs.operator const char*();
			}

			bool operator>(const std::string& lhs, const String& rhs)
			{
				return lhs > rhs.operator const char*();
			}

			bool operator>(const char* lhs, const String& rhs)
			{
				return lhs > rhs.operator const char*();
			}

			bool operator<=(const std::string& lhs, const String& rhs)
			{
				return lhs <= rhs.operator const char*();
			}

			bool operator<=(const char* lhs, const String& rhs)
			{
				return lhs <= rhs.operator const char*();
			}

			bool operator>=(const std::string& lhs, const String& rhs)
			{
				return lhs >= rhs.operator const char*();
			}

			bool operator>=(const char* lhs, const String& rhs)
			{
				return lhs >= rhs.operator const char*();
			}

			String& operator<<(String& lhs, const String& rhs)
			{
				lhs += rhs;
				return lhs;
			}

			String& operator<<(String& lhs, const std::string& rhs)
			{
				lhs += rhs;
				return lhs;
			}

			String& operator<<(String& lhs, const char* rhs)
			{
				lhs += rhs;
				return lhs;
			}

			String& operator<<(String& lhs, char rhs)
			{
				lhs += rhs;
				return lhs;
			}

			String& operator>>(String& lhs, String& rhs)
			{
				rhs = lhs;
				lhs.Clear();
				return lhs;
			}

			String& operator>>(String& lhs, char& rhs)
			{
				rhs = lhs[0];
				lhs.Remove((Size)0);
				return lhs;
			}

			const Size String::NotPosition = std::string::npos;
		}
	}
}

namespace std
{
	template <>
	struct formatter<RenderStar::Util::Core::String> : formatter<std::string>
	{
		auto format(const RenderStar::Util::Core::String& str, format_context& ctx) const
		{
			std::string message = str;
			return formatter<std::string>::format(message, ctx);
		}
	};
}