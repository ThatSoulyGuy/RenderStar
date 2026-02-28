#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Component/AuthorityContext.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Component/ComponentPool.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <string>

namespace RenderStar::Common::Component
{
    class ComponentModule final : public Module::AbstractModule
    {
    public:

        class Builder
        {
        public:

            Builder& Affector(std::unique_ptr<AbstractAffector> affector);
            std::unique_ptr<ComponentModule> Build();

        private:

            std::vector<std::unique_ptr<AbstractAffector>> affectors;
        };

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

        void RunAffectors();

        void SetEntityAuthority(GameObject entity, EntityAuthority authority);

        [[nodiscard]]
        EntityAuthority GetEntityAuthority(GameObject entity) const;

        [[nodiscard]]
        bool CheckAuthority(GameObject entity, const AuthorityContext& caller) const;

        template<typename ComponentType>
        std::optional<std::reference_wrapper<ComponentType>> GetComponentAuthorized(GameObject entity, const AuthorityContext& caller);

        template<typename ComponentType>
        ComponentType& AddComponentAuthorized(GameObject entity, const AuthorityContext& caller);

        template<typename ComponentType>
        ComponentType& AddComponentAuthorized(GameObject entity, ComponentType component, const AuthorityContext& caller);

        template<typename ComponentType>
        void RemoveComponentAuthorized(GameObject entity, const AuthorityContext& caller);

        void MarkEntityDirty(GameObject entity);

        std::unordered_set<int32_t> ConsumeDirtyEntities();

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        template<typename ComponentType>
        void EnsurePoolExists();

        int32_t nextEntityId;
        std::vector<GameObject> entities;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;
        ComponentPool<std::string> namePool;
        ComponentPool<EntityAuthority> authorityPool;
        std::unordered_set<int32_t> dirtyEntities;
    };
}

#include "RenderStar/Common/Component/ComponentModule.inl"
