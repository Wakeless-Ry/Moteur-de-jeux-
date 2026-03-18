#ifndef ECS_MANAGER
#define ECS_MANAGER

#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

#include "Component.h"
#include "System.h"
#include "utils.h"

class ECSManager {
    struct ComponentListBase {
        virtual ~ComponentListBase() = default;
        virtual void remove(EntityId entity) = 0;
    };

    template <typename T> struct ComponentList : ComponentListBase {
        std::array<T, ECS::MAX_ENTITIES> components;

        std::unordered_map<EntityId, size_t> indices;
        std::queue<size_t> empty_indices;

        template <typename U> void set(U &&component, EntityId entity) {
            if (this->empty_indices.empty()) {
                indices[entity] = components.size();
                components.push_back(std::forward<U>(component));
            } else {
                size_t index = this->empty_indices.front();
                components[index] = std::forward<U>(component);
                this->indices[entity] = index;
                this->empty_indices.pop();
            }
        }

        void remove(EntityId entity) override {
            auto it = this->indices.find(entity);
            if (it == this->indices.end())
                return;

            this->components[it->second] = T{};
            this->empty_indices.push(it->second);
            this->indices.erase(entity);
        }

        std::optional<std::reference_wrapper<const T>>
        get(EntityId entity) const {
            auto it = this->indices.find(entity);
            if (it == this->indices.end())
                return std::nullopt;
            return std::cref(this->components[it->second]);
        }
    };

    template <typename T> struct SignedManager {
        unsigned long counter = 0;
        std::unordered_set<T> unused;
        std::unordered_map<T, ECS::Signature> signatures;

        T generateId() {
            T element;
            if (unused.empty()) {
                element = T{++this->counter};
            } else {
                element = *this->unused.begin();
                this->unused.erase(this->unused.begin());
            }
            this->signatures[element] = ECS::Signature();
            return element;
        }

        void remove(T element) {
            this->signatures.erase(element);
            if (element.value < this->counter) {
                this->unused.insert(element);
            }
        }

        const ECS::Signature &getSignature(T element) const {
            return signatures.at(element);
        }

        const std::set<T>
        getAllCompatible(const ECS::Signature &signature) const {
            std::set<T> result;

            for (auto const &[element, other] : this->signatures) {
                if ((other & signature) == signature) {
                    result.insert(other);
                }
            }

            return result;
        }

        void registerComponent(T element, size_t index) {
            this->signatures.at(element).set(index);
        }

        void unregisterComponent(T element, size_t index) {
            this->signatures.at(element).reset(index);
        }
    };

    SignedManager<EntityId> entityManager;
    SignedManager<SystemId> systemManager;

    std::map<std::type_index, std::unique_ptr<ComponentListBase>> componentMap;

    std::unordered_map<std::type_index, size_t> componentIndices;

    std::unordered_map<SystemId, std::shared_ptr<System>> systems;

    ECSManager(const ECSManager &) = delete;
    ECSManager &operator=(const ECSManager &) = delete;

    template <typename T> void registerType(size_t index) {
        componentMap[std::type_index(typeid(T))] =
            std::make_unique<ComponentList<T>>();
        componentIndices[std::type_index(typeid(T))] = index;
    }

    template <typename Tuple, size_t... Is>
    void registerAll(std::index_sequence<Is...>) {
        (registerType<std::tuple_element_t<Is, Tuple>>(Is), ...);
    }

    ECSManager() {
        registerAll<ComponentTypes>(
            std::make_index_sequence<std::tuple_size_v<ComponentTypes>>{});
    };

    template <typename T> inline ComponentList<T> &getList() {
        return static_cast<ComponentList<T> &>(
            *componentMap.at(std::type_index(typeid(T))));
    }

    template <typename T> inline ComponentList<T> &getList() const {
        return this->getList<T>();
    }

    void entityComponentsChanged(EntityId entity) {
        ECS::Signature eSignature = this->entityManager.getSignature(entity);
        for (auto &[id, system] : this->systems) {
            ECS::Signature sSignature = this->systemManager.getSignature(id);

            if ((eSignature & sSignature) == sSignature) {
                system->addEntity(entity);
            } else {
                system->removeEntity(entity);
            }
        }
    }

  public:
    static ECSManager &getManager() {
        static ECSManager instance;
        return instance;
    }

    EntityId generateEntityId() {
        EntityId entity = this->entityManager.generateId();
        assert(entity.value < ECS::MAX_ENTITIES &&
               "Too many entities in existence.");
        return entity;
    }
    void removeEntity(EntityId entity) {
        for (auto &[_, list] : this->componentMap) {
            list->remove(entity);
        }
        this->entityManager.remove(entity);
    }

    template <typename T>
    void setComponentToEntity(T &&component, EntityId entity) {
        using C = std::remove_reference_t<T>;
        this->getList<C>().set(std::forward<T>(component), entity);
        this->entityManager.registerComponent(entity,
                                              componentIndices.at(typeid(C)));
        this->entityComponentsChanged(entity);
    }

    template <typename T> void removeComponentFromEntity(EntityId entity) {
        this->getList<T>().remove(entity);
        this->entityManager.unregisterComponent(entity,
                                                componentIndices.at(typeid(T)));
        this->entityComponentsChanged(entity);
    }

    template <typename T>
    std::optional<std::reference_wrapper<const T>>
    getComponentOfEntity(EntityId entity) const {
        return this->getList<T>().get(entity);
    }

    SystemId registerSystem(std::shared_ptr<System> system) {
        SystemId id = this->systemManager.generateId();
        this->systems[id] = system;
        return id;
    }

    template <typename T> void registerComponentToSystem(SystemId system) {
        this->systemManager.registerComponent(system,
                                              componentIndices.at(typeid(T)));
        ECS::Signature signature = this->systemManager.getSignature(system);
        std::set<EntityId> compatible =
            this->entityManager.getAllCompatible(signature);
        this->systems[system]->setEntities(compatible);
    }

    void removeSystem(SystemId system) {
        this->systemManager.remove(system);
        this->systems.erase(system);
    }
};

#endif // ECS_MANAGER