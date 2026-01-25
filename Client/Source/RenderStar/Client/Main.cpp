#include "RenderStar/Client/ClientApplication.hpp"

int main(const int argumentCount, char* argumentValues[])
{
    RenderStar::Client::ClientApplication application;
    return application.Run(argumentCount, argumentValues);
}
