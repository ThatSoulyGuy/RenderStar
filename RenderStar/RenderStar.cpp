#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/Core/Window.hpp"
#include "RenderStar/Engine.hpp"

using namespace RenderStar;
using namespace RenderStar::Core;

int main()
{
	Settings::GetInstance()->Set<String>("defaultDomain", "RenderStar");

	Engine::GetInstance()->PreInitialize();

	Window::GetInstance()->Create();
	Window::GetInstance()->Show();
	
	Engine::GetInstance()->Initialize();

	Window::GetInstance()->AddUpdateFunction([]() 
	{ 
		Engine::GetInstance()->Update();
		Engine::GetInstance()->Render();	
	});

	Window::GetInstance()->Run();

	Engine::GetInstance()->CleanUp();

	return 0;
}