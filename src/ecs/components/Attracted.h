#ifndef ATTRACTED
#define ATTRACTED

#include "src/ecs/utils.h"
#include <vector>

enum AttractionMode {
    INWARD,
    OUTWARD,
};

struct Attraction {
    EntityId body;
    AttractionMode mode;
    float force;

    Attraction(EntityId body, AttractionMode mode, float force)
        : body(body), mode(mode), force(force) {}
};

class Attracted {

    std::vector<Attraction> attractions;

  public:
    std::vector<Attraction> const &getAttractions() const {
        return this->attractions;
    }

    void addAttraction(EntityId body, AttractionMode mode, float force) {
        this->attractions.push_back({body, mode, force});
    }

    void clear() { this->attractions.clear(); }
};

#endif // ATTRACTED