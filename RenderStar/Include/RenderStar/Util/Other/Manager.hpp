#pragma once

#include "RenderStar/Util/Core/Map.hpp"
#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;
using namespace RenderStar::Util::Core;

#define MakeManager(managementType, managedType) \
	class managementType \
	{ \
 \
	public: \
 \
		managementType(const managementType&) = delete; \
		managementType& operator=(const managementType&) = delete; \
 \
		void Register(Shared<managedType> type) \
		{ \
			registeredTypes[type->GetName()] = type; \
		} \
 \
		void Unregister(const String& name) \
		{ \
			registeredTypes.Remove(name); \
		} \
 \
		Shared<managedType> Get(const String& name) \
		{ \
			return registeredTypes[name]; \
		} \
 \
		void CleanUp() \
		{ \
			for (auto& [name, type] : registeredTypes) \
				type->CleanUp(); \
		} \
 \
		static Shared<managementType> GetInstance() \
		{ \
			class Enabled : public managementType { }; \
			static Shared<managementType> instance = std::make_shared<Enabled>(); \
 \
			return instance; \
		} \
 \
	private: \
 \
		ShaderManager() = default; \
		 \
		Map<String, Shared<Shader>> registeredTypes; \
 \
	}; 