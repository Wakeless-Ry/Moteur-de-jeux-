#include "ECSManager.h"
#include <cassert>

ECSManager::ECSManager() {
    registerAll<ComponentTypes>(
        std::make_index_sequence<std::tuple_size_v<ComponentTypes>>{});
}

ECSManager &ECSManager::getManager() {
    static ECSManager instance;
    return instance;
}

EntityId ECSManager::generateEntityId() {
    EntityId entity = this->entityManager.generateId();
    assert(entity.value < ECS::MAX_ENTITIES &&
           "Too many entities in existence.");
    return entity;
}

void ECSManager::removeEntity(EntityId entity) {
    for (auto &[_, list] : this->componentMap) {
        list->remove(entity);
    }
    this->entityManager.remove(entity);
}

void ECSManager::entityComponentsChanged(EntityId entity) {
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

SystemId ECSManager::registerSystem(std::shared_ptr<System> system) {
    SystemId id = this->systemManager.generateId();
    this->systems[id] = system;
    return id;
}

void ECSManager::removeSystem(SystemId system) {
    this->systemManager.remove(system);
    this->systems.erase(system);
}