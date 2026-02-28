#pragma once

#include <cstdint>

namespace RenderStar::Common::Component
{
    enum class AuthorityLevel : uint8_t
    {
        NOBODY = 0,
        SERVER = 1,
        CLIENT = 2
    };

    struct EntityAuthority
    {
        AuthorityLevel level = AuthorityLevel::NOBODY;
        int32_t ownerId = -1;

        static constexpr EntityAuthority Nobody() { return { AuthorityLevel::NOBODY, -1 }; }
        static constexpr EntityAuthority Server() { return { AuthorityLevel::SERVER, -1 }; }
        static constexpr EntityAuthority Client(int32_t clientId) { return { AuthorityLevel::CLIENT, clientId }; }

        [[nodiscard]]
        constexpr bool CanModify(AuthorityLevel callerLevel, int32_t callerId) const
        {
            if (level == AuthorityLevel::NOBODY)
                return true;

            if (level == AuthorityLevel::SERVER)
                return callerLevel == AuthorityLevel::SERVER;

            if (level == AuthorityLevel::CLIENT)
                return callerLevel == AuthorityLevel::CLIENT && callerId == ownerId;

            return false;
        }
    };
}
