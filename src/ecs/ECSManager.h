#ifndef ECS_MANAGER
#define ECS_MANAGER

#include <map>
#include <memory>
#include <typeindex>
#include <vector>

#include "Component.h"
#include "Entity.h"
#include "System.h"
#include <optional>
#include <queue>

typedef unsigned long ulong;

struct EntityId {
    unsigned long value;
    bool operator<(const EntityId &other) const { return value < other.value; }
};

struct ComponentId {
    unsigned long value;
    bool operator<(const ComponentId &other) const {
        return value < other.value;
    }
};

struct SystemId {
    unsigned long value;
    bool operator<(const SystemId &other) const { return value < other.value; }
};

class ECSManager {
    struct ComponentListBase {
        virtual ~ComponentListBase() = default;
    };

    template <typename T> struct ComponentList : ComponentListBase {
        std::vector<T> components;

        std::map<EntityId, size_t> indices;
        std::queue<size_t> empty_indices;

        template <typename U> void add(U &&component, EntityId entity) {
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

        void remove(EntityId entity) {
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

    unsigned long eCounter = 0;
    unsigned long cCounter = 0;
    unsigned long sCounter = 0;

    std::map<std::type_index, std::unique_ptr<ComponentListBase>> componentMap;

    ECSManager(const ECSManager &) = delete;
    ECSManager &operator=(const ECSManager &) = delete;

    template <typename T> void registerType() {
        componentMap[std::type_index(typeid(T))] =
            std::make_unique<ComponentList<T>>();
    }

    template <typename Tuple, size_t... Is>
    void registerAll(std::index_sequence<Is...>) {
        (registerType<std::tuple_element_t<Is, Tuple>>(), ...);
    }

    ECSManager() {
        registerAll<ComponentTypes>(
            std::make_index_sequence<std::tuple_size_v<ComponentTypes>>{});
    };

    template <typename T> ComponentList<T> &getList() {
        return static_cast<ComponentList<T> &>(
            *componentMap.at(std::type_index(typeid(T))));
    }

  public:
    static ECSManager &getManager() {
        static ECSManager instance;
        return instance;
    }

    EntityId generateEntityId() { return EntityId{++eCounter}; }
    ComponentId generateComponentId() { return ComponentId{++cCounter}; }
    SystemId generateSystemId() { return SystemId{++sCounter}; }

    template <typename T>
    void addComponentToEntity(T &&component, EntityId entity) {
        this->getList<std::remove_reference_t<T>>().add(
            std::forward<T>(component), entity);
    }

    template <typename T> void removeComponentFromEntity(EntityId entity) {
        this->getList<T>().remove(entity);
    }

    template <typename T>
    std::optional<std::reference_wrapper<const T>> get(EntityId entity) const {
        return this->getList<T>().get(entity);
    }
};

#endif // ECS_MANAGER