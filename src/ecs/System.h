#ifndef SYSTEM
#define SYSTEM

#include <set>

#include "utils.h"

class System {
    std::set<EntityId> entities;

  public:
    void addEntity(EntityId id) { this->entities.insert(id); }

    void removeEntity(EntityId id) { this->entities.erase(id); }

    void setEntities(const std::set<EntityId> &ids) { this->entities = ids; }
};

#endif // SYSTEM