#pragma once

#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/General/CommonVersionFormat.hpp"
#include "RenderStar/Util/Other/Buildable.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util::Core;
using namespace RenderStar::Util::General;
using namespace RenderStar::Util::Other;

namespace RenderStar
{
	namespace Mod
	{
		struct ModRegistration : public Buildable<ModRegistration> 
		{
			String registryName;
			String displayName;
			String description;
			String author;
			CommonVersionFormat version;

			ModRegistration SetRegistryName(const String& registryName)
			{
				this->registryName = registryName;

				return *this;
			}

			ModRegistration SetDisplayName(const String& displayName)
			{
				this->displayName = displayName;

				return *this;
			}

			ModRegistration SetDescription(const String& description)
			{
				this->description = description;

				return *this;
			}

			ModRegistration SetAuthor(const String& author)
			{
				this->author = author;

				return *this;
			}

			ModRegistration SetVersion(const CommonVersionFormat& version)
			{
				this->version = version;

				return *this;
			}

			ModRegistration Build()
			{
				return *this;
			}

			static ModRegistration New()
			{
				return ModRegistration();
			}
		};

		class IMod
		{

		public:
			
			IMod() = default;
			
			virtual ~IMod() = default;

			IMod(const IMod&) = delete;
			IMod& operator=(const IMod&) = delete;

			virtual void PreInitialize()
			{
				ModRegistration registration = GetRegistration();

				registryName = registration.registryName;
				displayName = registration.displayName;
				description = registration.description;
				author = registration.author;
				version = registration.version;
			}

			virtual void Initialize() = 0;
			virtual void Update() = 0;
			virtual void Render() = 0;
			virtual void CleanUp() = 0;

			String GetRegistryName() const
			{
				return registryName;
			}

			String GetDisplayName() const
			{
				return displayName;
			}

			String GetDescription() const
			{
				return description;
			}

			String GetAuthor() const
			{
				return author;
			}

			CommonVersionFormat GetVersion() const
			{
				return version;
			}

			virtual ModRegistration GetRegistration() const = 0;

		private:
			
			String registryName;
			String displayName;
			String description;
			String author;
			CommonVersionFormat version;

		};
	}
}