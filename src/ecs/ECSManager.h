#ifndef ECS_MANAGER
#define ECS_MANAGER

#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <typeindex>
#include <unordered_set>
#include <vector>

#include "Component.h"
#include "Entity.h"
#include "System.h"

struct EntityId {
    ulong value;
    bool operator<(const EntityId &other) const { return value < other.value; }
    bool operator==(const EntityId &other) const {
        return value == other.value;
    }
};

struct SystemId {
    ulong value;
    bool operator<(const SystemId &other) const { return value < other.value; }
    bool operator==(const SystemId &other) const {
        return value == other.value;
    }
};

namespace std {
template <> struct hash<EntityId> {
    size_t operator()(const EntityId &id) const noexcept {
        return std::hash<ulong>{}(id.value);
    }
};
template <> struct hash<SystemId> {
    size_t operator()(const SystemId &id) const noexcept {
        return std::hash<ulong>{}(id.value);
    }
};
} // namespace std

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

    template <typename T> struct IdManager {
        ulong counter = 0;
        std::unordered_set<T> unused;

        T generateId() {
            if (unused.empty()) {
                return T{++this->counter};
            } else {
                T front = *this->unused.begin();
                this->unused.erase(this->unused.begin());
                return front;
            }
        }

        void removeId(T id) {
            if (id.value < this->counter) {
                this->unused.insert(id);
            }
        }
    };

    IdManager<EntityId> eIdManager;
    IdManager<SystemId> sIdManager;

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

    EntityId generateEntityId() { return this->eIdManager.generateId(); }
    void removeEntityId(EntityId id) { return this->eIdManager.removeId(id); }

    SystemId generateSystemId() { return this->sIdManager.generateId(); }
    void removeSystemId(SystemId id) { return this->sIdManager.removeId(id); }

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