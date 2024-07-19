#pragma once

#include "RenderStar/Util/String.hpp"
#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
	namespace Util
	{
		template <typename Key, typename Value>
		class Map
		{

		public:
		
			Map() = default;

			Map(const Map& other)
			{
				*this = other;
			}

			Map(Map&& other) noexcept
			{
				*this = std::move(other);
			}

			Map(const std::unordered_map<Key, Value>& other)
			{
				*this = other;
			}

			Map(std::unordered_map<Key, Value>&& other) noexcept
			{
				*this = std::move(other);
			}

			Map(std::initializer_list<std::pair<const Key, Value>> list)
			{
				*this = list;
			}

			Map(const std::map<Key, Value>& other)
			{
				for (const auto& pair : other)
					data[pair.first] = pair.second;
			}

			Map& operator=(const Map& other)
			{
				if (this != &other)
				{
					data = other.data;
				}

				return *this;
			}

			Map& operator=(Map&& other) noexcept
			{
				if (this != &other)
				{
					data = std::move(other.data);
				}

				return *this;
			}

			Map& operator=(const std::unordered_map<Key, Value>& other)
			{
				data = other;
				return *this;
			}

			Map& operator=(std::unordered_map<Key, Value>&& other) noexcept
			{
				data = std::move(other);
				return *this;
			}

			Map& operator=(std::initializer_list<std::pair<const Key, Value>> list)
			{
				data = list;
				return *this;
			}

			Map& operator+=(const Map& other)
			{
				for (const auto& pair : other.data)
					data[pair.first] = pair.second;

				return *this;
			}

			Map& operator+=(const std::unordered_map<Key, Value>& other)
			{
				for (const auto& pair : other)
					data[pair.first] = pair.second;

				return *this;
			}

			Map& operator+=(std::unordered_map<Key, Value>&& other)
			{
				for (auto& pair : other)
					data[pair.first] = std::move(pair.second);

				return *this;
			}

			Map& operator+=(std::initializer_list<std::pair<const Key, Value>> list)
			{
				for (const auto& pair : list)
					data[pair.first] = pair.second;

				return *this;
			}

			Map operator+(const Map& other) const
			{
				Map result = *this;
				result += other;
				return result;
			}

			Map operator+(const std::unordered_map<Key, Value>& other) const
			{
				Map result = *this;
				result += other;
				return result;
			}

			Map operator+(std::unordered_map<Key, Value>&& other) const
			{
				Map result = *this;
				result += std::move(other);
				return result;
			}

			Map operator+(std::initializer_list<std::pair<const Key, Value>> list) const
			{
				Map result = *this;
				result += list;
				return result;
			}

			Map& operator-=(const Map& other)
			{
				for (const auto& pair : other.data)
					data.erase(pair.first);

				return *this;
			}

			Map& operator-=(const std::unordered_map<Key, Value>& other)
			{
				for (const auto& pair : other)
					data.erase(pair.first);

				return *this;
			}

			Map& operator-=(std::unordered_map<Key, Value>&& other)
			{
				for (auto& pair : other)
					data.erase(pair.first);

				return *this;
			}

			Map& operator-=(std::initializer_list<std::pair<const Key, Value>> list)
			{
				for (const auto& pair : list)
					data.erase(pair.first);

				return *this;
			}

			Map operator-(const Map& other) const
			{
				Map result = *this;
				result -= other;
				return result;
			}

			Map operator-(const std::unordered_map<Key, Value>& other) const
			{
				Map result = *this;
				result -= other;
				return result;
			}

			Map operator-(std::unordered_map<Key, Value>&& other) const
			{
				Map result = *this;
				result -= std::move(other);
				return result;
			}

			Map operator-(std::initializer_list<std::pair<const Key, Value>> list) const
			{
				Map result = *this;
				result -= list;
				return result;
			}

			Value& operator[](const Key& key)
			{
				return data[key];
			}

			const Value& operator[](const Key& key) const
			{
				return data.at(key);
			}

			bool Contains(const Key& key) const
			{
				return data.contains(key);
			}

			Size Length() const
			{
				return data.size();
			}

			auto begin() const
			{
				return data.begin();
			}

			auto end() const
			{
				return data.end();
			}

			bool IsEmpty() const
			{
				return data.empty();
			}

			void Remove(const Key& key)
			{
				data.erase(key);
			}

			void Clear()
			{
				data.clear();
			}

		private:

			std::unordered_map<Key, Value> data;

		};
	}
}