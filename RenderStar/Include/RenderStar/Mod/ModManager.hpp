#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Mod/Mod.hpp"
#include "RenderStar/Util/Core/Map.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Util::Core;

namespace RenderStar
{
	namespace Mod
	{
		using CreateModInstance = IMod* (*)();

		class ModManager
		{

		public:

			ModManager(const ModManager&) = delete;
			ModManager& operator=(const ModManager&) = delete;

			void LoadFromDirectory(const String& directory) const
			{
				if (!std::filesystem::exists(directory.operator std::string()))
					std::filesystem::create_directory(directory.operator std::string());

				Vector<String> files;

				for (const auto& entry : std::filesystem::directory_iterator(directory.operator std::string()))
					files += entry.path().string();

				if (files.Length() == 0)
				{
					Logger_WriteConsole("No mods found in directory: '" + directory + "'.", LogLevel::INFORMATION);
					return;
				}
				
				for (const String& file : files)
				{
					HMODULE hModule = LoadLibrary(file);

					if (!hModule) 
						return;
					
					CreateModInstance createInstance = (CreateModInstance)GetProcAddress(hModule, "CreateModInstance");

					if (!createInstance) 
					{
						FreeLibrary(hModule);

						Logger_ThrowException("Failed to find mod activation function from mod: '" + file + "'.", false);

						return;
					}

					IMod* mod = createInstance();

					if (mod) 
					{
						mod->PreInitialize();

						registeredMods[mod->GetRegistryName()] = mod;
						loadedModules[mod->GetRegistryName()] = hModule;

						mod->Initialize();
					}
					else
					{
						Logger_ThrowException("Failed to create mod instance from mod: '" + file + "'.", false);
						FreeLibrary(hModule);
					}
				}
			}

			void Update()
			{
				for (const auto& [name, mod] : registeredMods)
					mod->Update();
			}

			void Render()
			{
				for (const auto& [name, mod] : registeredMods)
					mod->Render();
			}

			void CleanUp()
			{
				for(const auto& [name, mod] : registeredMods) 
				{
					mod->CleanUp();
					FreeLibrary(loadedModules[name]);
				}

				registeredMods.Clear();
				loadedModules.Clear();
			}

			static Shared<ModManager> GetInstance()
			{
				class Enabled : public ModManager { };
				static Shared<ModManager> instance = std::make_shared<Enabled>();

				return instance;
			}
			
		private:

			ModManager() = default;

			mutable Map<String, IMod*> registeredMods;
			mutable Map<String, HMODULE> loadedModules;
		};

		EXPORT_INSTANCE_FUNCTION(ModManager);
	}
}