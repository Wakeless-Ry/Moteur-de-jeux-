#ifndef WRITE_CSV
#define WRITE_CSV

#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/RigidBody.h"
#include "src/ecs/utils.h"
#include <fstream>
#include <optional>
#include <string>

using namespace std;

class Write_CSV : public System {
  private:
    ofstream csv;

  public:
    Write_CSV() : csv("./assets/csv/out.csv") {}

    void update(float deltaTime) override {

        ECSManager &ecs = ECSManager::getManager();
        for (EntityId entity : this->getEntities()) {
            auto pos_ecs = ecs.getComponentOfEntity<Positionable>(entity);
            auto velocity_ecs = ecs.getComponentOfEntity<RigidBody>(entity);
            auto pos = pos_ecs.value().get().pos;
            auto velocity = velocity_ecs.value().get().velocity;
            string tmp =
                to_string(entity.value) + "," + to_string(deltaTime) + ",";
            tmp += to_string(pos.x) + "," + to_string(pos.y) + "," +
                   to_string(pos.z) + ",";
            tmp += to_string(velocity.x) + "," + to_string(velocity.y) + "," +
                   to_string(velocity.z) + "\n";
            this->csv << tmp;
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::getManager().registerComponentToSystem<Positionable>(id);
        ECSManager::getManager().registerComponentToSystem<RigidBody>(id);
    }

    ~Write_CSV() { csv.close(); };
};

#endif