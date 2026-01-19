#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Component/ComponentPool.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <optional>
#include <string>

namespace RenderStar::Common::Component
{
    using ComponentSystemFunction = std::function<void(class ComponentModule&)>;

    class ComponentModule : public Module::AbstractModule
    {
    public:

        ComponentModule();

        GameObject CreateEntity();

        GameObject CreateEntity(const std::string& name);

        void DestroyEntity(GameObject entity);

        bool EntityExists(GameObject entity) const;

        std::optional<std::reference_wrapper<std::string>> GetEntityName(GameObject entity);

        std::optional<GameObject> FindEntityByName(const std::string& name);

        template<typename ComponentType>
        ComponentType& AddComponent(GameObject entity);

        template<typename ComponentType>
        ComponentType& AddComponent(GameObject entity, ComponentType component);

        template<typename ComponentType>
        void RemoveComponent(GameObject entity);

        template<typename ComponentType>
        std::optional<std::reference_wrapper<ComponentType>> GetComponent(GameObject entity);

        template<typename ComponentType>
        bool HasComponent(GameObject entity) const;

        template<typename ComponentType>
        ComponentPool<ComponentType>& GetPool();

        void RegisterSystem(ComponentSystemFunction system);

        void RunSystems();

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        template<typename ComponentType>
        void EnsurePoolExists();

        int32_t nextEntityId;
        std::vector<GameObject> entities;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;
        ComponentPool<std::string> namePool;
        std::vector<ComponentSystemFunction> systems;
    };
}

#include "RenderStar/Common/Component/ComponentModule.inl"
