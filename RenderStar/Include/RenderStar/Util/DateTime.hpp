#pragma once

#include "RenderStar/Util/String.hpp"
#include "RenderStar/Util/Typedefs.hpp"

namespace RenderStar
{
    namespace Util
    {
        class DateTime
        {

        public:

            static String Get(const String& format)
            {
                Time now = std::time(nullptr);
                TimeInformation information;

                localtime_s(&information, &now);

                OutputStringStream buffer{};

                char dateTime[100];

                Size length = format.Length();

                for (Size i = 0; i < length; ++i)
                {
                    if (format[i] == '%' && i + 1 < length)
                    {
                        char specifier = format[i + 1];
                        String specifierFormat = '%' + String(specifier);

                        std::strftime(dateTime, sizeof(dateTime), specifierFormat, &information);
                        buffer << dateTime;

                        ++i;
                    }
                    else
                        buffer << format[i];
                }

                return buffer.str();
            }
        };
    }
}