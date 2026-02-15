#ifndef SCENE
#define SCENE

#include <map>
#include <optional>
#include <set>
#include <stack>
#include <vector>

#include "Mesh.hpp"
#include "Transform.hpp"

class Scene;

typedef size_t NodeId;

class BasicNode {
    Transform transformOfNode;

    void transformNode(const Transform &transform) {
        // TODO Apply transform to this->transformOfNode
    }

    friend class Scene;

  public:
    BasicNode() {}
    BasicNode(const Transform &transform) : transformOfNode(transform) {}
};

class Scene {
    class SceneTree {
        std::map<NodeId, std::set<NodeId>> tree;
        std::map<NodeId, NodeId> parents;

        void removeFromParents(NodeId node) {
            if (this->parents.count(node)) {
                NodeId parent = this->parents[node];
                this->parents.erase(node);
                this->tree[parent].erase(node);
            }
        }

        void addChildNodeUnsafe(NodeId parent, NodeId node) {
            this->tree[parent].insert(node);
            this->parents[node] = parent;
        }

      public:
        SceneTree() {}

        bool hasNode(NodeId node) const { return this->tree.count(node); }

        bool addNode(NodeId node) {
            if (!this->hasNode(node)) {
                this->tree[node];
                return true;
            }

            return false;
        }

        bool addChildNode(NodeId parent, NodeId node) {
            if (this->hasNode(parent) && this->addNode(node)) {
                this->addChildNodeUnsafe(parent, node);
                return true;
            }

            return false;
        }

        bool changeParent(NodeId parent, NodeId node) {
            if (this->hasNode(parent), this->hasNode(node)) {
                this->removeFromParents(node);
                this->addChildNodeUnsafe(parent, node);
                return true;
            }

            return false;
        }

        std::vector<NodeId> getAllChildren(NodeId node) {
            std::vector<NodeId> result;

            if (this->hasNode(node)) {
                std::stack<NodeId> toProcess;

                toProcess.push(node);

                while (!toProcess.empty()) {
                    NodeId current = toProcess.top();
                    toProcess.pop();

                    result.push_back(current);

                    if (this->hasNode(current)) {
                        std::set<NodeId> children = this->tree[current];
                        for (auto it = children.begin(); it != children.end();
                             ++it) {
                            toProcess.push(*it);
                        }
                    }
                }
            }

            return result;
        }

        std::vector<NodeId> remove(NodeId node) {
            std::vector<NodeId> toRemove = this->getAllChildren(node);

            for (const NodeId &id : toRemove) {
                this->removeFromParents(id);
                this->tree.erase(id);
            }

            return toRemove;
        }

        void printTree() {
            for (auto it = this->tree.begin(); it != this->tree.end(); ++it) {
                std::cout << it->first << ": ";

                for (auto it2 = it->second.begin(); it2 != it->second.end();
                     ++it2) {
                    std::cout << (*it2) << " ";
                }

                std::cout << endl;
            }
        }
    };

    template <typename Content> struct NodeList {
        std::map<NodeId, size_t> idNodeMap;
        std::vector<Content> nodes;
        std::vector<bool> flags;

        void addNode(NodeId id, Content node) {
            this->nodes.push_back(node);
            this->flags.push_back(true);
            this->idNodeMap[id] = nodes.size() - 1;
        }

        std::optional<Content *> getNode(NodeId id) {
            if (this->idNodeMap.count(id) && this->flags[this->idNodeMap[id]]) {
                return &this->nodes[this->idNodeMap[id]];
            }
            return std::nullopt;
        }

        bool removeNode(NodeId id) {
            if (this->idNodeMap.count(id) && this->flags[this->idNodeMap[id]]) {
                this->flags[this->idNodeMap[id]] = false;
                return true;
            }
            return false;
        }

        bool hasId(NodeId id) { return this->idNodeMap.count(id); }
    };

    NodeId idCpt = 1;

    SceneTree tree;
    BasicNode root;
    static const NodeId ROOT_ID = 0;

    NodeList<Mesh> meshList;
    NodeList<BasicNode> basicNodeList;

    NodeId newId() {
        this->idCpt++;
        return this->idCpt - 1;
    }

    void transform(const Transform &transform,
                   const std::vector<NodeId> &nodes) {
        for (const NodeId &id : nodes) {
            if (this->meshList.hasId(id)) {
                std::optional<Mesh *> node = this->meshList.getNode(id);
                if (node.has_value()) {
                    node.value()->transformNode(transform);
                }
            } else if (this->basicNodeList.hasId(id)) {
                std::optional<BasicNode *> node =
                    this->basicNodeList.getNode(id);
                if (node.has_value()) {
                    node.value()->transformNode(transform);
                }
            }
        }
    }

  public:
    Scene() { this->tree.addNode(ROOT_ID); }

    void cleanUp() {}

    void transform(const Transform &transform) {
        this->transform(transform, ROOT_ID);
    }

    void transform(const Transform &transform, NodeId id) {
        this->transform(transform, this->tree.getAllChildren(id));
    }

    void transformOnlyPropagation(const Transform &transform, NodeId id) {
        std::vector<NodeId> descendants = this->tree.getAllChildren(id);
        descendants.erase(descendants.begin());
        this->transform(transform, descendants);
    }

    void transformWithoutPropagation(const Transform &transform, NodeId id) {
        if (this->meshList.hasId(id)) {
            std::optional<Mesh *> node = this->meshList.getNode(id);
            if (node.has_value()) {
                node.value()->transformNode(transform);
            }
        } else if (this->basicNodeList.hasId(id)) {
            std::optional<BasicNode *> node = this->basicNodeList.getNode(id);
            if (node.has_value()) {
                node.value()->transformNode(transform);
            }
        }
    }

    void draw(const Camera &camera) {
        for (size_t i = 0; i < this->meshList.flags.size(); i++) {
            if (this->meshList.flags[i]) {
                this->meshList.nodes[i].draw(camera);
            }
        }
    }

    NodeId addMesh(Mesh mesh) {
        NodeId newId = this->newId();
        this->meshList.addNode(newId, mesh);

        this->tree.addChildNode(ROOT_ID, newId);

        return newId;
    }

    std::optional<NodeId> addMeshAsChild(NodeId parent, Mesh mesh) {
        if (this->meshList.hasId(parent) || this->basicNodeList.hasId(parent)) {
            NodeId newId = this->addMesh(mesh);

            this->changeParent(parent, newId);

            return newId;
        }

        return std::nullopt;
    }

    std::optional<Mesh *> getMesh(NodeId id) {
        return this->meshList.getNode(id);
    }

    NodeId addBasicNode(BasicNode basicNode) {
        NodeId newId = this->newId();
        this->basicNodeList.addNode(newId, basicNode);

        this->tree.addChildNode(ROOT_ID, newId);

        return newId;
    }

    std::optional<NodeId> addBasicNodeAsChild(NodeId parent,
                                              BasicNode basicNode) {
        if (this->meshList.hasId(parent) || this->basicNodeList.hasId(parent)) {
            NodeId newId = this->addBasicNode(basicNode);

            this->changeParent(parent, newId);

            return newId;
        }

        return std::nullopt;
    }

    std::optional<BasicNode *> getBasicNode(NodeId id) {
        return this->basicNodeList.getNode(id);
    }

    bool setSceneAsParent(NodeId id) {
        return this->tree.changeParent(ROOT_ID, id);
    }

    bool changeParent(NodeId newParent, NodeId child) {
        return this->tree.changeParent(newParent, child);
    }

    bool removeNode(NodeId id) {
        std::vector<NodeId> removed = this->tree.remove(id);

        for (const NodeId &id : removed) {
            if (this->meshList.hasId(id)) {
                this->meshList.removeNode(id);
            } else if (this->basicNodeList.hasId(id)) {
                this->basicNodeList.removeNode(id);
            }
        }

        return removed.size();
    }

    void printTree() { this->tree.printTree(); }
};

#endif // SCENE