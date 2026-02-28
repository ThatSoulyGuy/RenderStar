#pragma once

#include "RenderStar/Common/Component/EntityAuthority.hpp"

namespace RenderStar::Common::Component
{
    struct AuthorityContext
    {
        AuthorityLevel level = AuthorityLevel::NOBODY;
        int32_t ownerId = -1;

        static constexpr AuthorityContext AsServer() { return { AuthorityLevel::SERVER, -1 }; }
        static constexpr AuthorityContext AsClient(int32_t clientId) { return { AuthorityLevel::CLIENT, clientId }; }
        static constexpr AuthorityContext AsNobody() { return { AuthorityLevel::NOBODY, -1 }; }
    };
}
