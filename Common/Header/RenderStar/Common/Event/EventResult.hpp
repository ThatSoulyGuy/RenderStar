#pragma once

#include <string>

namespace RenderStar::Common::Event
{
    enum class EventResultType
    {
        SUCCESS,
        FAILURE,
        FATAL
    };

    struct EventResult
    {
        EventResultType type;
        std::string message;

        static EventResult Success()
        {
            return { EventResultType::SUCCESS, {} };
        }

        static EventResult Failure(std::string message)
        {
            return { EventResultType::FAILURE, std::move(message) };
        }

        static EventResult Fatal(std::string message)
        {
            return { EventResultType::FATAL, std::move(message) };
        }
    };
}
