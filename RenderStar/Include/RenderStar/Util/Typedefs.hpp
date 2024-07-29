#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <tuple>
#include <array>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <list>
#include <deque>
#include <variant>
#include <optional>
#include <any>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <limits>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <regex>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <filesystem>
#include <format>
#include <memory>
#include <any>
#include <variant>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <cctype>
#include <wrl.h>
#include <wrl/client.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef ERROR
#undef WriteConsole
#undef DOMAIN

#ifdef IS_MOD
#define MOD_API __declspec(dllexport)
#else
#define MOD_API __declspec(dllimport)
#endif // IS_MOD

#ifdef IS_MOD
#define MOD_API_N __declspec(dllimport)
#else
#define MOD_API_N __declspec(dllexport)
#endif // IS_MOD

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#define EXPORT_INSTANCE_FUNCTION(className) extern "C" DLL_EXPORT Shared<className> className##_GetInstance() \
{ \
	return className::GetInstance(); \
}

namespace RenderStar
{
	namespace Util
	{
		typedef std::int8_t int8;
		typedef std::int16_t int16;
		typedef std::int32_t int32;
		typedef std::int64_t int64;
		
		typedef std::uint8_t uint8;
		typedef std::uint16_t uint16;
		typedef std::uint32_t uint32;
		typedef std::uint64_t uint64;

		typedef std::size_t Size;

		typedef std::time_t Time;
		typedef std::tm TimeInformation;

		typedef std::ostream OutputStream;
		typedef std::basic_ostream<wchar_t, std::char_traits<wchar_t>> WOutputStream;
		typedef std::istream InputStream;

		typedef std::ostringstream OutputStringStream;
		typedef std::istringstream InputStringStream;

		typedef std::fstream FileStream;
		typedef std::ifstream InputFileStream;

		typedef std::regex RegularExpression;

		typedef std::any Any;

		typedef std::mutex Mutex;
		typedef std::condition_variable ConditionVariable;
		typedef std::thread Thread;	

		typedef std::type_index TypeIndex;
		typedef std::type_info TypeInformation;

		typedef std::chrono::system_clock SystemClock;

		typedef std::chrono::high_resolution_clock HighResolutionClock;

		typedef std::chrono::steady_clock SteadyClock;

		typedef std::chrono::milliseconds Milliseconds;
		typedef std::chrono::seconds Seconds;
		typedef std::chrono::minutes Minutes;
		typedef std::chrono::hours Hours;

		typedef std::chrono::time_point<SystemClock> TimePoint;
		typedef std::chrono::duration<float> Duration;

		template<typename T>
		using Shared = std::shared_ptr<T>;

		template<typename T>
		using Unique = std::unique_ptr<T>;

		template<typename T>
		using Weak = std::weak_ptr<T>;

		template<typename T>
		using Function = std::function<T>;

		template<typename T>
		using LockGuard = std::lock_guard<T>;

		template<typename T>
		using Atomic = std::atomic<T>;

		template<typename T>
		using Optional = std::optional<T>;

		template<typename T>
		using Match = std::match_results<T>;

		template<typename... Args>
		using Tuple = std::tuple<Args...>;

		template<typename... Args>
		using Variant = std::variant<Args...>;

		template<typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
	}
}