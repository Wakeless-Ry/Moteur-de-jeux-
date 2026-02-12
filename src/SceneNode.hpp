#ifndef SCENE
#define SCENE

#include <set>
#include <functional>

#include <src/Transform.hpp>

class SceneNode {
    struct RefCompare {
        bool operator()(const std::reference_wrapper<SceneNode>& a,
                        const std::reference_wrapper<SceneNode>& b) const {
            return &a.get() < &b.get();
        }
    };

    std::set<std::reference_wrapper<SceneNode>, RefCompare> children;

public:
    SceneNode() {}

    virtual void transform(const Transform &transform) = 0;

    bool addChild(SceneNode &node) {
        return this->children.insert(std::ref(node)).second;
    }

    bool remove(SceneNode &node) {
        for (auto it = this->children.begin(); it != this->children.end(); ++it) {
            if (&(it->get()) == &node) {
                this->children.erase(it);
                return true;
            }
        }
        return false;
    }

    bool contains(const SceneNode &node) const {
        for (const auto& ref : this->children) {
            if (&(ref.get()) == &node) {
                return true;
            }
        }
        return false;
    }

    void transformAndPropagate(const Transform &transform) {
        this->transform(transform);
        for (SceneNode &child : this->children) {
            child.transformAndPropagate(transform);
        }
    }
};

#endif //MESH