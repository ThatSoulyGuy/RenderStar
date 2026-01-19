#pragma once

#include <memory>

namespace RenderStar::Common::Module
{
    class ModuleManager;
}

namespace RenderStar::Client
{
    class ClientApplication
    {
    public:

        ClientApplication();

        ~ClientApplication();

        int Run(int argumentCount, char* argumentValues[]);

    private:

        void Initialize();

        void Shutdown();

        std::unique_ptr<Common::Module::ModuleManager> moduleManager;
    };
}
