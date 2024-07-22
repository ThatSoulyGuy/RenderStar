#pragma once

#include "RenderStar/Util/General/DateTime.hpp"
#include "RenderStar/Util/General/Formatter.hpp"

#define Logger_WriteConsole(message, level) _Logger::WriteConsole(message, __FUNCTION__, level)
#define Logger_ThrowException(message, fatal) _Logger::ThrowException(message, __FUNCTION__, __LINE__, fatal)
#define Logger_ThrowIfFailed(message, result) if (FAILED(result)) _Logger::ThrowException(F("{} with result: '{}'", message, #result), __FUNCTION__, __LINE__, false)

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Core
	{
		enum class LogLevel
        {
            INFORMATION,
            DEBUGGING,
            WARNING,
            ERROR,
            FATAL_ERROR
        };

        class _Logger
        {

        public:

            static void WriteConsole(const String& message, const String& function, const LogLevel& level)
            {
                static String name = GetFunctionName(function);
                name[0] = toupper(name[0]);

                switch (level)
                {

                case LogLevel::INFORMATION:
                    std::cout << Formatter::Format(Formatter::ColorFormat("&2&l[{}] [Thread/INFORMATION] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
                    break;

                case LogLevel::DEBUGGING:
                    std::cout << Formatter::Format(Formatter::ColorFormat("&1&l[{}] [Thread/DEBUGGING] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
                    break;

                case LogLevel::WARNING:
                    std::cout << Formatter::Format(Formatter::ColorFormat("&6[{}] [Thread/WARNING] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
                    break;

                case LogLevel::ERROR:
                    std::cout << Formatter::Format(Formatter::ColorFormat("&c[{}] [Thread/ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
                    break;

                case LogLevel::FATAL_ERROR:
                    std::cout << Formatter::Format(Formatter::ColorFormat("&4[{}] [Thread/FATAL ERROR] [{}]: {}"), DateTime::Get("%H:%S:%M"), name, message) << std::endl;
                    break;

                default:
                    break;
                }
            }

            static void ThrowException(const String& message, const String& function, int line, bool fatal)
			{
				static String name = GetFunctionName(function);
				name[0] = toupper(name[0]);

				if (fatal)
				{
					std::cout << Formatter::Format(Formatter::ColorFormat("&4&l[{}] [Thread/FATAL ERROR] [{}]: {} [Line: {}]"), DateTime::Get("%H:%S:%M"), name, message, line) << std::endl;
					
                    MessageBeep(MB_ICONERROR);

                    MessageBox(NULL, message, "Fatal Error", MB_ICONERROR | MB_OK);
				}
				else
					std::cout << Formatter::Format(Formatter::ColorFormat("&c[{}] [Thread/ERROR] [{}]: {} [Line: {}]"), DateTime::Get("%H:%S:%M"), name, message, line) << std::endl;
			}

        private:

            static String GetFunctionName(const String& input)
            {
                Size position = input.Find("::");

                if (position != String::NotPosition)
                    position += 2;
                else
                    position = 0;

                return input.SubString(position);
            }

        };
	}
}